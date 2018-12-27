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

#include "cache.h"
#include "daemon.h"

int main(int argc, char* argv[]) {
  pname = argv[0];
  // default ssd location
  int opt;
  uint8_t operation = NOOP;
  string object_name;
  while ((opt = getopt(argc, argv, "s:GMKn:ra:Wh?p:g:")) != -1) {
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
    log << "Wiped entire superblock!" << endl;
    if (close(ssd_fd) < 0) {
      whine << "Cannot close " << ssd_devname << ": " << strerror(errno) << endl;
      exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
  }

  // get ssd and cache infos, then overwrite ssd superblock
  if (reset) {
    get_attributes_from_dev();
    print_setup_attributes();

    // reset the attributes in the superblock
    struct superblock* sb = new superblock();

    if (write_superblock((char*)sb, sizeof(struct superblock)) < 0) {
      whine << "Cannot reset ssd superblock " << ssd_devname << ": " << strerror(errno) << endl;
      exit(EXIT_FAILURE);
    }
    delete sb;

    // reset all sets
    for (int i = 0; i < cache_set_count; i++) {
      if (write_metadata_set(i, nullptr) < 0) {
        whine << "Cannot reset metadata set " << i << ": " << strerror(errno) << endl;
        exit(EXIT_FAILURE);
      }
    }
  }

  else {

    // initialize daemon
    std::shared_ptr<cache_daemon> daemon(new cache_daemon);
    daemon->initialize();

    if (!object_name.length() && operation != NOOP) {
      whine << "No object given" << endl;
      exit(EXIT_FAILURE);
    }
    // for (auto i : daemon->sets) i->print();

    if (operation != NOOP) {
      boost::filesystem::path object_path(object_name);
      std::vector<string> v;
      boost::split(v, object_path.filename().string(), [] (char c) { return c == '.'; });
      // ensure object name contains 3 parts
      if (v.size() != 3) {
        whine << "Invalid object name" << endl;
        exit(EXIT_FAILURE);
      }

      std::shared_ptr<cache_metadata_entry> entry(new cache_metadata_entry);
      entry->initialize(v);
      // entry->print();

      auto set_id = entry->object_id % cache_set_count;
      // the metadata set
      auto cache_set = daemon->sets[set_id];
      // cache_set->print();

      switch (operation) {
        case PUT:
          {
            log << "PUT" << endl;
            // check if object exists
            if (!boost::filesystem::exists(object_path)) {
              whine << object_path << " does not exist" << endl;
              exit(EXIT_FAILURE);
            }
            int ret = cache_set->insert(daemon, entry);
            if (ret < 0) {
              whine << "Failed to insert " << object_path << endl;
              exit(EXIT_FAILURE);
            }
            if (ret > 0) {
              whine << object_path << " is already in cache" << endl;
              exit(EXIT_FAILURE);
            }
            auto idx = entry->index;
            log << "inserting to daemon->entries[" << idx << "]" << endl;
            assert(idx < daemon->entries.size());
            break;
          }
        case GET:
          {
            log << "GET" << endl;
            int ret = cache_set->retrieve(daemon, entry);
            if (ret) {
              whine << "Failed to get " << object_path << endl;
              exit(EXIT_FAILURE);
            }
            // TODO: now, entry is the result of our query!
            break;
          }
        default:
          {
            whine << "unknown operation" << endl;
            exit(EXIT_FAILURE);
            break;
          }
      }
      // update metadata here!
      if (write_metadata_set(set_id, cache_set) < 0) {
        whine << "Cannot reset metadata set " << set_id << ": " << strerror(errno) << endl;
        exit(EXIT_FAILURE);
      }
    }
    // we're just gonna do nothing
    else {
      log << "no op" << endl;
    }
    // for debugging
    //daemon->print();
  }

  if (close(ssd_fd) < 0) {
    whine << "Cannot close " << ssd_devname << ": " << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  }
  return EXIT_SUCCESS;
}
