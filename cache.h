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

#ifndef CACHE_H
#define CACHE_H

#include "common.h"

struct superblock {
  char device_name[DEV_PATHLEN]; // TODO: use uuid
  uint32_t sector_size; // bytes per sector
  uint64_t device_size; // in sectors
  uint32_t block_size; // in sectors, stands for size of cache data chunk
  uint32_t object_size; // in sectors, stands for actual RBD object size
  uint64_t entries; // depends on block_size
  uint64_t associativity; // how large is a cache set
                          // this value equals entries if only one set is present,
                          // i.e. fully associative
  uint16_t sets;
  uint64_t md_len; // in sectors
  uint64_t data_len; // in sectors

  superblock();
  void print();
};

struct cache_metadata_set { // size = 1 sector
  uint8_t set_id;
  uint32_t lru_head, lru_tail; // index of entries at LRU's head / tail
  uint64_t lru_size;
  uint32_t invalid_head; // index of next invalid block. 0 if doesn't exist
  uint64_t PBA_begin, PBA_end; // boundaries of PBAs of data managed by this set
  uint32_t checksum;
  bool unclean; // set if metadata and data are mismatched

  void print();
};

struct cache_metadata_entry { // size = 1 sector
  char image_id[16];
  char object_id[16];
  uint32_t index; // which sector is this entry on
  bool valid_bit;
  uint64_t PBA; // physical block address of data chunk
  uint32_t lru_prev, lru_next; // these denote neighboring LRU entries
  uint32_t prev, next; // these denote neighboring available entries in set

  void print();
};

#endif
