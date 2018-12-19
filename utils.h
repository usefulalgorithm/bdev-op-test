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

#endif
