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

#ifndef UTILS_H
#define UTILS_H
#include "common.h"

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

void set_set_count() {
  cache_set_count = 1 + (cache_entries-1)/cache_associativity;
}

uint64_t get_max_cache_entries() {
  return (ssd_dev_size - 1 - cache_set_count) / (ssd_block_size + 1);
}

void get_parts_lengths() {
  // three parts
  //
  // 1. superblock
  //  already set
  // 2. metadata
  metadata_length = cache_set_count + cache_entries;
  // 3. data
  data_length = ssd_dev_size - metadata_length;
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

// Invoked on reset
/*
void wipe_metadata(int fd) {
  auto length = cache_set_count + cache_entries
  char empty_buf[513];
  std::fill(len, buffer+512, '\0');
}
*/

#endif
