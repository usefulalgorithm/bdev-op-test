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
  char device_name[512];
  uint32_t sector_size; // how many bytes per sector
  uint64_t device_size; // in sectors
  uint32_t block_size; // in sectors
  uint32_t md_block_size; // in sectors
  uint32_t object_size; // in sectors
  uint64_t entries;
  uint64_t tree_size; // in sectors
  void print();
};

#endif
