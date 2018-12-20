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

void superblock::print() {
  log << "device_name: " << device_name
    << ", sector_size=" << sector_size
    << ", device_size=" << device_size
    << ", block_size=" << block_size
    << ", object_size=" << object_size
    << ", entries=" << entries 
    << ", associativity=" << associativity
    << ", sets=" << sets
    << ", md_len=" << md_len
    << ", data_len=" << data_len << endl;
}

void cache_metadata_set::print() {
  log << "set_id=" << set_id
    << ", lru_head=" << lru_head
    << ", lru_tail=" << lru_tail
    << ", invalid_head=" << invalid_head
    << ", PBA_begin=" << PBA_begin
    << ", PBA_end=" << PBA_end
    << ", checksum=" << checksum
    << ", unclean=" << unclean << endl;
}

void cache_metadata_entry::print() {
  log << "image_id=" << image_id
    << ", object_id=" << object_id
    << ", index=" << index
    << ", valid_bit=" << valid_bit
    << ", PBA=" << PBA
    << ", lru_prev=" << lru_prev 
    << ", lru_next=" << lru_next 
    << ", prev=" << prev 
    << ", next=" << next << endl;
}

