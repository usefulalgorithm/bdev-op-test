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

#include "daemon.h"

void cache_daemon::initialize() {
  // superblock
  sb = std::make_shared<superblock>();

  if (read_superblock(reinterpret_cast<char*>(sb.get())) < 0) {
    whine << "Cannot read SSD superblock " << ssd_devname << ": " << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  }
  sb->get_attributes();

  if (!strlen(sb->device_name)) {
    whine << "No ssd device given. Please reset cache superblock first." << endl;
    exit(EXIT_FAILURE);
  }

  // metadata sets
  for (int i = 0; i < cache_set_count; i++) {
    std::shared_ptr<cache_metadata_set> md_set((cache_metadata_set*)malloc(sizeof(cache_metadata_set)), free);
    if (read_metadata_set(i, md_set) < 0) {
      whine << "Cannot read metadata set " << i << ": " << strerror(errno) << endl;
      exit(EXIT_FAILURE);
    }
    sets.push_back(md_set);
  }

  // metadata set entries
  entries.resize(cache_set_count * cache_associativity);
  for (int i = 0; i < cache_set_count; i++) {
    auto cur = sets[i]->lru_head;
    while (cur != CACHE_NULL) {
      std::shared_ptr<cache_metadata_entry> entry(new cache_metadata_entry);
      read_metadata_entry(cur, entry);
      auto idx = entry->offset - superblock_length - i -1;
      cur = entry->lru_next;
      entries[idx] = std::move(entry);
    }
  }
}

void cache_daemon::print() {
  debug("===== Superblock =====");
  sb->print();
  debug("===== Metadata sets =====");
  for (size_t i = 0; i < sets.size(); i++) {
    debug("----- set " + std::to_string(i) + " -----");
    sets[i]->print();
    for (size_t j = 0; j < cache_associativity; j++) {
      auto idx = j+cache_associativity*i;
      if (entries[idx] != nullptr) {
        debug("..... entry " + std::to_string(idx) + " .....");
        entries[idx]->print();
      }
    }
  }
}
