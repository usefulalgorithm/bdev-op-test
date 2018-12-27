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
#include "utils.h"

struct cache_daemon;

struct superblock {
  char device_name[DEV_PATHLEN]; // TODO: use uuid
  uint16_t sets;
  uint32_t sector_size; // bytes per sector
  uint32_t block_size; // in sectors, stands for size of cache data chunk
  uint32_t object_size; // in sectors, stands for actual RBD object size
  uint64_t device_size; // in sectors
  uint64_t entries; // depends on block_size
  uint64_t associativity; // how large is a cache set
                          // this value equals entries if only one set is present,
                          // i.e. fully associative
  uint64_t md_len; // in sectors
  uint64_t data_len; // in sectors

  superblock();
  void print();
  void get_attributes();
};

struct cache_metadata_entry { // size = 1/4 sector
  bool valid_bit;
  hash_t pool_id;
  hash_t image_id;
  hash_t object_id;
  uint32_t index;
  uint32_t lru_prev, lru_next; // these denote neighboring LRU entries
  uint32_t prev, next; // these denote neighboring available entries in set
  uint64_t PBA; // physical block address of data chunk

  void print();
  void initialize(std::vector<string>);
};

struct cache_metadata_set { // size = 1 sector
  uint8_t set_id;
  bool unclean; // set if metadata and data are mismatched
  uint32_t checksum;
  uint32_t lru_head, lru_tail; // index of entries at LRU's head / tail
  uint32_t invalid_head; // index of next invalid block. 0 if doesn't exist
  uint64_t lru_size; // how many entries are in this set
  uint64_t cache_size; // how many entries can this set store
  uint64_t PBA_begin, PBA_end; // boundaries of PBAs of data managed by this set, in bytes

  cache_metadata_set(int);
  void print();

  int insert(std::shared_ptr<cache_daemon>, std::shared_ptr<cache_metadata_entry>);
  int lookup(std::shared_ptr<cache_daemon>, std::shared_ptr<cache_metadata_entry>, size_t&, uint32_t&);
  int retrieve(std::shared_ptr<cache_daemon>, std::shared_ptr<cache_metadata_entry>&);
  int evict(std::shared_ptr<cache_metadata_entry>);
}; // __attribute__((packed));

int write_superblock(char* buf, size_t len);
int read_superblock(char* buf);

int write_metadata_set(int set_id, std::shared_ptr<cache_metadata_set> md_set);
int reset_metadata_entries(uint32_t offset, std::shared_ptr<cache_metadata_set> md_set);
int read_metadata_set(int set_id, std::shared_ptr<cache_metadata_set> md_set);

int write_metadata_entry(std::shared_ptr<cache_metadata_entry> md_entry);
uint32_t get_metadata_entry_offset(std::shared_ptr<cache_metadata_entry> md_entry);
int read_metadata_entry(uint32_t index, std::shared_ptr<cache_metadata_entry> md_entry);

#endif
