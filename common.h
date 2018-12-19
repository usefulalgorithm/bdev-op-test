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
#define DEV_PATHLEN   128
#define NOOP          0
#define PUT           1
#define GET           2
#define EVICT         3

#define SEED          0xbabeface

typedef uint32_t hash_t;
typedef uint32_t sector_t;

static const int buf_size = 512;
static const int part_sb_size = 1;
static const int part_btree_size = 128;
static string ssd_devname("/dev/sdf2");
static string pname;

#endif
