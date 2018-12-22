/*
 *  bdev-op-test: A simple implementation of a block-based cache
 *  Copyright (C) 2018  Tsung-Ju Lii <usefulalgorithm@gmail.com>
 *
 *  This file is part of bdev-op-test.
 *
 *  bdev-op-test is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  bdev-op-test is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with bdev-op-test.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>

#include "SpookyV2.h"
// use map to store location of the data
// (hashed val of obj name) ===> (location on disk)
#include "stx/btree_map.h"

#include "cache.h"
#include "utils.h"
namespace io = boost::iostreams;

void write_btree(int fd, stx::btree_map<hash_t, sector_t>& bmap) {
  lseek(fd, 512, SEEK_SET);
  io::file_descriptor_sink fds(fd, io::never_close_handle);
  io::stream_buffer<io::file_descriptor_sink> sb(fds);
  std::ostream os(&sb);
  bmap.dump(os);
#ifdef DEBUG
  long pos = os.tellp();
  log << "on disk pos=" << pos << endl;
#endif
}

void read_btree(int fd, stx::btree_map<hash_t, sector_t>& bmap) {
  lseek(fd, 512, SEEK_SET);
  io::file_descriptor_source fds(fd, io::never_close_handle);
  io::stream_buffer<io::file_descriptor_source> sb(fds);
  std::istream is(&sb);
  if (!bmap.restore(is)) {
    whine << "Cannot restore b+ tree" << endl;
    exit(EXIT_FAILURE);
  }
}

int main(int argc, char* argv[]) {
  pname = argv[0];
  // default ssd location
  int opt, ssd_fd;
  uint8_t operation = NOOP;
  string object_name;
  while ((opt = getopt(argc, argv, "s:GMKn:ra:Wh?p:g:e:")) != -1) {
    switch (opt) {
      case 's':
        ssd_devname = optarg;
        break;
      case 'a':
        try {
          cache_associativity = std::stoull(optarg);
        } catch (const std::exception& e) {
          whine << "Invalid cache associativity" << endl;
          usage(pname);
        }
      case 'r':
        reset = true;
        break;
      case 'p':
        if (operation != NOOP) {
          cerr << "Can only do one operation at a time" << endl;
          usage(pname);
        }
        operation = PUT;
        object_name = optarg;
        break;
      case 'g':
        if (operation != NOOP) {
          cerr << "Can only do one operation at a time" << endl;
          usage(pname);
        }
        operation = GET;
        object_name = optarg;
        break;
      case 'e':
        if (operation != NOOP) {
          cerr << "Can only do one operation at a time" << endl;
          usage(pname);
        }
        operation = EVICT;
        object_name = optarg;
        break;
      case 'W':
        wipe_superblock = true;
        break;
      case 'G':
        base_size_str = "GB";
        base_size = 1 << 30;
        break;
      case 'M':
        base_size_str = "MB";
        base_size = 1 << 20;
        break;
      case 'K':
        base_size_str = "KB";
        base_size = 1 << 10;
        break;
      case 'n':
        try {
          cache_entries = std::stoull(optarg);
        } catch (const std::exception& e) {
          whine << "Invalid number of cache entries." << endl;
          usage(pname);
        }
        break;
      case '?':
      case 'h':
      default:
        usage(pname);
    }
  }
  ssd_fd = open(ssd_devname.c_str(), O_RDWR);
  if (ssd_fd < 0) {
    whine << "Cannot open " << ssd_devname << ": " << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  }
  lseek(ssd_fd, 0, SEEK_SET);

  // wipe everything!
  if (wipe_superblock) {
    char buffer[512];
    std::fill(buffer, buffer+512, '\0');
    if (write(ssd_fd, buffer, 512) < 0) {
      whine << "Cannot wipe out ssd superblock " << ssd_devname << ": " << strerror(errno) << endl;
      exit(EXIT_FAILURE);
    }
#ifdef DEBUG
    log << "Wiped entire superblock!" << endl;
#endif
    if (close(ssd_fd) < 0) {
      whine << "Cannot close " << ssd_devname << ": " << strerror(errno) << endl;
      exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
  }

  // get ssd and cache infos, then overwrite ssd superblock
  if (reset) {
    get_attributes_from_dev(ssd_fd);
#ifdef DEBUG
    print_setup_attributes();
#endif
    
    // reset the attributes in the superblock
    struct superblock* sb = new superblock();

    if (write_superblock(ssd_fd, (char*)sb, sizeof(struct superblock)) < 0) {
      whine << "Cannot reset ssd superblock " << ssd_devname << ": " << strerror(errno) << endl;
      exit(EXIT_FAILURE);
    }
    delete sb;

    // reset all sets
    for (int i = 0; i < cache_set_count; i++) {
      if (write_metadata_set(ssd_fd, i) < 0) {
        whine << "Cannot reset metadata set " << i << ": " << strerror(errno) << endl;
        exit(EXIT_FAILURE);
      }
    }
  }

  // read existing superblock
  else {
    char sb_buffer[512];
    if (read_superblock(ssd_fd, sb_buffer) < 0) {
      whine << "Cannot read SSD superblock " << ssd_devname << ": " << strerror(errno) << endl;
      exit(EXIT_FAILURE);
    }
    struct superblock* sb = (struct superblock*)sb_buffer;
#ifdef DEBUG
    sb->print();
#endif
    sb->get_attributes();

    if (!strlen(sb->device_name)) {
      whine << "No ssd device given. Please reset cache superblock first." << endl;
      exit(EXIT_FAILURE);
    }
    if (!object_name.length() && operation != NOOP) {
      whine << "No object given" << endl;
      exit(EXIT_FAILURE);
    }

    // retrieve all sets
    std::vector<cache_metadata_set*> sets;
    for (int i = 0; i < cache_set_count; i++) {
#if 1
      auto md_set = (cache_metadata_set*)malloc(sizeof(cache_metadata_set));
#else
      cache_metadata_set* md_set = nullptr;
#endif
      if (read_metadata_set(ssd_fd, i, md_set) < 0) {
        whine << "Cannot read metadata set " << i << ": " << strerror(errno) << endl;
        exit(EXIT_FAILURE);
      }
      sets.push_back(md_set);
    }
    if (operation != NOOP) {
      /*
      // restore b+ tree here
      read_btree(ssd_fd, bmap);
      hash_t hashed = SpookyHash::Hash32(object_name.c_str(), object_name.length(), SEED);
#ifdef DEBUG
      log << "hashed=" << hashed << endl;
#endif
      // the actual location on disk
      sector_t sector = 0;

      switch (operation) {
        case PUT:
          {
            // check if object exists
            if (!std::experimental::filesystem::exists(object_name)) {
              whine << object_name << " does not exist" << endl;
              exit(EXIT_FAILURE);
            }
            auto ret = bmap.insert(std::pair<hash_t, sector_t>(hashed, sector));
#ifdef DEBUG
            if (ret.second == false)
              log << "object already in btree, ignoring" << endl;
#endif
            // TODO write data onto disk
            break;
          }
        case GET:
          {
            auto ret = bmap.find(hashed);
            if (ret == bmap.end()) {
              whine << "object not in btree, exiting" << endl;
              exit(EXIT_FAILURE);
            }
            // TODO get data from disk
            break;
          }
        case EVICT:
          {
            auto ret = bmap.find(hashed);
            if (ret == bmap.end()) {
              whine << "object not in btree, exiting" << endl;
              exit(EXIT_FAILURE);
            }
            bmap.erase(ret);
            break;
          }
        default:
          {
            whine << "unknown operation" << endl;
            exit(EXIT_FAILURE);
            break;
          }
      }

      // dump b+ tree here
      write_btree(ssd_fd, bmap);
      */
    }
    // we're just gonna do nothing
    else {
#ifdef DEBUG
      log << "no op" << endl;
#endif
    }
  }

  if (close(ssd_fd) < 0) {
    whine << "Cannot close " << ssd_devname << ": " << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  }
  return EXIT_SUCCESS;
}
