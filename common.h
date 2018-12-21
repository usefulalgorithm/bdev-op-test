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

#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <algorithm>
#include <string>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <limits>
#include <exception>
#include <experimental/filesystem>
#include "fcntl.h"
#include "unistd.h"
#include <sys/ioctl.h>
#include <linux/fs.h>

using std::string;
using std::cout;
using std::cerr;
using std::endl;

#define whine (cerr << "[ERROR]\t" << __func__ << ": ")
#define log   (cout << "[INFO]\t" <<  __func__ << ": ")
#define DEV_PATHLEN   32
#define NOOP          0
#define PUT           1
#define GET           2
#define EVICT         3

#define SEED          0xbabeface
#define CACHE_NULL    0xFFFFFFFF
#define MD_SET_LEN    (1 + cache_associativity) * ssd_sector_size

typedef uint32_t hash_t;
typedef uint32_t sector_t;


// static variables, lifetime spans the entire program

extern string   pname;
extern string   ssd_devname;
extern uint64_t base_size;
extern string   base_size_str;
extern uint64_t ssd_dev_size;
extern uint32_t ssd_sector_size;
extern uint32_t ssd_block_size;
extern uint32_t cache_obj_size;
extern uint64_t cache_entries;
extern uint64_t cache_associativity;
extern uint16_t cache_set_count;

static const uint16_t max_cache_set_count = std::numeric_limits<uint16_t>::max();

// lengths

static const uint8_t  superblock_length = 1;
extern uint64_t       metadata_length;
extern uint64_t       data_length;

// operations

extern bool reset;
extern bool wipe_superblock;

#endif
