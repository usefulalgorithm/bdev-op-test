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

#include "common.h"

string   pname                = ""; // program name
string   ssd_devname          = "/dev/sdf4";     // device name
uint64_t base_size            = 1;
string   base_size_str        = "B";          // defaults to Bytes
uint64_t ssd_dev_size         = 0;
uint32_t ssd_sector_size      = 0;
uint32_t ssd_block_size       = 8;           // defaults to 4K
uint32_t cache_obj_size       = 8*1024;  // defaults to 4M
uint64_t cache_entries        = 0;
uint64_t cache_associativity  = 0;
uint16_t cache_set_count      = 0;

uint64_t metadata_length      = 0;
uint64_t data_length          = 0;

bool reset = false;
bool wipe_superblock = false;
