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

#include "utils.h"

void usage(string pname) {
  cerr << "Usage: " << pname << " [-s SSD_LOCATION] [-K | -M | -G] [-n CACHE_ENTRIES] [-a ASSOCIATIVITY] [-r] [-W] [-h | -?] <-p OBJ | -g OBJ | -e OBJ>" << endl;
  cerr << "Options:" << endl;
  cerr << "\t-s\t\tsets location of the SSD. If unspecified, the default value is " << ssd_devname << endl;
  cerr << "\t-K | -M | -G\tsets base for displaying volume size" << endl;
  cerr << "\t-n\t\tsets number of cache entries" << endl;
  cerr << "\t-a\t\tsets cache associativity, fully associative if not set" << endl;
  cerr << "\t-r\t\tresets the cache" << endl;
  cerr << "\t-W\t\twipes out the cache superblock" << endl;
  cerr << "\t-h, -?\t\tprints this help message" << endl;
  cerr << "\t-p OBJ\t\tputs OBJ into the cache" << endl;
  cerr << "\t-g OBJ\t\tgets OBJ from the cache" << endl;
  cerr << "\t-e OBJ\t\tevicts OBJ from the cache" << endl;
  exit(EXIT_FAILURE);
}

string check_if_null(uint32_t x) {
  if (x == CACHE_NULL)
      return std::string("NULL");
    else
      return std::to_string(x);
}

void set_set_count() {
  cache_set_count = 1 + (cache_entries-1)/cache_associativity;
}

uint64_t get_max_cache_entries() {
  return 4*(ssd_dev_size - 1 - cache_set_count) / (4*cache_obj_size + 1);
}

void get_parts_lengths() {
  // three parts
  //
  // 1. superblock
  //  already set
  // 2. metadata
  metadata_length = cache_set_count * (1 + cache_associativity/4);
  // 3. data
  data_length = cache_obj_size * cache_entries;
}

void get_attributes_from_dev(int fd) {
  if (ioctl(fd, BLKGETSIZE, &ssd_dev_size) < 0) {
    whine << "Cannot get SSD device size: " << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  }
  if (ioctl(fd, BLKSSZGET, &ssd_sector_size) < 0) {
    whine << "Cannot get SSD sector size: " << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  }
  if (!cache_entries) {
    whine << "Invalid number of cache entries." << endl;
    usage(pname);
  }

  if (cache_associativity == 0) // did not specify associativity, assume fully associated
    cache_associativity = cache_entries;
  assert(cache_associativity > 1);
  if (cache_associativity % 4) {
    whine << "Cache associativity has to be multiple of 4 for alignment purposes" << endl;
    exit(EXIT_FAILURE);
  }
  set_set_count();

  if (cache_set_count > max_cache_set_count) {
    whine << "Maximum set number exceeded. The limit is " << max_cache_set_count << endl;
    exit(EXIT_FAILURE);
  }

  auto max_cache_entries = get_max_cache_entries();
  if (cache_entries > max_cache_entries) {
    whine << "Maximum entry number exceeded. The limit is " << max_cache_entries << endl;
    exit(EXIT_FAILURE);
  }

  get_parts_lengths();
}

void print_setup_attributes() {
  std::stringstream ss;
  ss << "device name = " << ssd_devname
    << ", device size = " << (ssd_dev_size * 512)/base_size << base_size_str
    << ", sector size = " << ssd_sector_size << "B"
    << ", block size = " << ssd_block_size*512 << "B" 
    << ", object size = " << cache_obj_size*512/base_size << base_size_str
    << ", cache entries = " << cache_entries
    << ", cache associativity = " << cache_associativity
    << ", cache set count = " << cache_set_count
    << ", superblock length = " << int(superblock_length) << " sectors"
    << ", metadata length = " << metadata_length << " sectors"
    << ", data length = " << data_length << " sectors" << endl;
  debug(ss.str());
}
