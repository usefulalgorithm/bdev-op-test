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
#include "daemon.h"

superblock::superblock() {
  strncpy(device_name, ssd_devname.c_str(), DEV_PATHLEN);
  sector_size = ssd_sector_size;
  device_size = ssd_dev_size;
  block_size = ssd_block_size;
  object_size = cache_obj_size;
  entries = cache_entries;
  associativity = cache_associativity;
  sets = cache_set_count;
  md_len = metadata_length;
  data_len = data_length;
}

void superblock::print() {
  std::stringstream ss;
  ss << "device_name: " << device_name
    << ", sector_size=" << sector_size << "B"
    << ", device_size=" << device_size << "sec."
    << ", block_size=" << block_size << "sec."
    << ", object_size=" << object_size << "sec."
    << ", entries=" << entries
    << ", associativity=" << associativity
    << ", sets=" << sets
    << ", md_len=" << md_len << "sec."
    << ", data_len=" << data_len << "sec." << endl;
  debug(ss.str());
}

cache_metadata_set::cache_metadata_set(int _set_id) {
  set_id = _set_id;
  lru_head = CACHE_NULL, lru_tail = CACHE_NULL; // cache_metadata_entry.offset
  lru_size = 0;
  cache_size = (cache_entries / cache_associativity - set_id) ? cache_associativity : cache_entries % cache_associativity;
  invalid_head = cache_associativity*(set_id);
  auto _PBA_begin = 1 + metadata_length + (_set_id) * cache_obj_size * cache_associativity;
  PBA_begin = _PBA_begin;
  PBA_end = _PBA_begin + cache_obj_size * cache_size;
  PBA_begin *= ssd_sector_size;
  PBA_end *= ssd_sector_size;
  checksum = 0;
  unclean = false;
  //debug("cache_size=" + std::to_string(cache_size));
}

void superblock::get_attributes() {
  ssd_devname = std::string(device_name);
  ssd_sector_size = sector_size;
  ssd_dev_size = device_size;
  ssd_block_size = block_size;
  cache_obj_size = object_size;
  cache_entries = entries;
  cache_associativity = associativity;
  cache_set_count = sets;
  metadata_length = md_len;
  data_length = data_len;
}

void cache_metadata_set::print() {
  std::stringstream ss;
  ss << "set_id=" << int(set_id)
    << ", lru_head=" << ::check_if_null(lru_head)
    << ", lru_tail=" << ::check_if_null(lru_tail)
    << ", lru_size=" << lru_size
    << ", cache_size=" << cache_size
    << ", invalid_head=" << ::check_if_null(invalid_head)
    << ", PBA_begin=" << PBA_begin << "B"
    << ", PBA_end=" << PBA_end << "B"
    << ", checksum=" << checksum
    << ", unclean=" << unclean << endl;
  debug(ss.str());
}

// returns 0 on success
//         1 if object already in cache
//         -1 on error
int cache_metadata_set::insert(std::shared_ptr<cache_daemon> daemon, std::shared_ptr<cache_metadata_entry> entry) {
  uint32_t pos, placeholder;
  pos = set_id*cache_associativity;
  placeholder = 0;
  if (lookup(daemon, entry, pos, placeholder) > 0)
    return 1;
  if (invalid_head == CACHE_NULL) // cache is full, do eviction here!
    evict(daemon);

  entry->index = invalid_head;
  if (lru_head == CACHE_NULL) {
    debug("insert to empty lru");
    lru_head = entry->index;
    assert(lru_tail == CACHE_NULL);
    lru_tail = entry->index;
    lru_size++;
    entry->PBA = PBA_begin;
    entry->valid_bit = true;
  }
  else {
    debug("insert to lru head");
    // get lru head
    auto cur_head = daemon->entries[lru_head];
    lru_head = entry->index;
    entry->lru_next = cur_head->index;
    cur_head->lru_prev = entry->index;
    write_metadata_entry(cur_head);
    lru_size++;
    entry->PBA = PBA_begin+(entry->index%cache_size)*cache_obj_size*ssd_sector_size;
    entry->valid_bit = true;
  }
  write_metadata_entry(entry);
  auto idx = entry->index;
  daemon->entries[idx] = std::move(entry);
  // check if this set is full
  if (lru_size == cache_size)
    invalid_head = CACHE_NULL;
  else
    invalid_head = pos;
  return 0;
}

// returns 0 if not found
//         1 if found
//         -1 on error
//
// pos is the next invalid_head
int cache_metadata_set::lookup(std::shared_ptr<cache_daemon> daemon,
    std::shared_ptr<cache_metadata_entry> entry, uint32_t& pos, uint32_t& index) {
  int ret = 0;
  auto cur = lru_head;
  while (cur != CACHE_NULL) {
    index = cur;
    auto cur_entry = daemon->entries[index];
    if (cur_entry->pool_id == entry->pool_id
        && cur_entry->image_id == entry->image_id
        && cur_entry->object_id == entry->object_id)
      return 1;
    pos = std::max(pos, index+1);
    cur = cur_entry->lru_next;
  }
  pos++;
  return ret;
}

// return 0 on success
//        1 if not found
int cache_metadata_set::retrieve(std::shared_ptr<cache_daemon> daemon, std::shared_ptr<cache_metadata_entry>& entry) {
  uint32_t pos = 0;
  uint32_t index = CACHE_NULL;
  if (lookup(daemon, entry, pos, index) == 0) // if not found, return
    return 1;
  if (index != lru_head) {
    // daemon->entries[index] is our target
    // first update its neighbors
    auto lru_prev_i = daemon->entries[index]->lru_prev, lru_next_i = daemon->entries[index]->lru_next;
    if (lru_prev_i != CACHE_NULL) {
      daemon->entries[lru_prev_i]->lru_next = lru_next_i;
      write_metadata_entry(daemon->entries[lru_prev_i]);
    }
    if (lru_next_i != CACHE_NULL) {
      daemon->entries[lru_next_i]->lru_prev = lru_prev_i;
      write_metadata_entry(daemon->entries[lru_next_i]);
    }
    if (index == lru_tail)
      lru_tail = lru_prev_i;
    // then update itself and the head
    daemon->entries[lru_head]->lru_prev = index;
    write_metadata_entry(daemon->entries[lru_head]);
    daemon->entries[index]->lru_next = lru_head;
    daemon->entries[index]->lru_prev = CACHE_NULL;
    write_metadata_entry(daemon->entries[index]);
    lru_head = index;
  }
  entry = daemon->entries[index];
  return 0;
}

int cache_metadata_set::evict(std::shared_ptr<cache_daemon> daemon) {
  auto entry = daemon->entries[lru_tail];
  entry->print();
  invalid_head = lru_tail;
  lru_tail = entry->lru_prev;
  daemon->entries[lru_tail]->lru_next = CACHE_NULL;
  lru_size --;
  write_metadata_entry(daemon->entries[lru_tail]);
  return 0;
}

void cache_metadata_entry::print() {
  std::stringstream ss;
  ss << "pool_id=" << pool_id
    << ", image_id=" << image_id
    << ", object_id=" << object_id
    << ", index=" << index
    << ", valid_bit=" << (valid_bit ? "VALID" : "INVALID")
    << ", PBA=" << PBA << "B"
    << ", lru_prev=" << check_if_null(lru_prev)
    << ", lru_next=" << check_if_null(lru_next)
    << ", prev=" << check_if_null(prev)
    << ", next=" << check_if_null(next) << endl;
  debug(ss.str());
}

void cache_metadata_entry::initialize(std::vector<string> v) {
  //XXX: actually, these fields *SHOULD* be integers when we get an object
  pool_id = SpookyHash::Hash32(v[0].c_str(), v[0].length(), SEED);
  image_id = SpookyHash::Hash32(v[1].c_str(), v[1].length(), SEED);
  object_id = std::stoul(v[2]);
  lru_prev = lru_next = prev = next = CACHE_NULL;
  PBA = 0;
  index = CACHE_NULL;
  valid_bit = false;
}


/*****************************************************
 *
 *  Global functions
 *
 *****************************************************/

int write_superblock(char* buf, size_t len) {
  // TODO are there other ways to do this?
  return write(ssd_fd, buf, len);
}

int read_superblock(char* buf) {
  // TODO are there other ways to do this?
  return read(ssd_fd, buf, sizeof(superblock));
}

int write_metadata_set(int set_id, std::shared_ptr<cache_metadata_set> md_set) {
  // get metadata set offset
  //
  // cache_associativity = number of entries in previous sets
  // length of one entry = 1/4 sector
  // length of one metadata set info = 1 sector
  //
  // offset = (length of superblock)
  //        + (which set is this)
  //          * (length of one metadata set info
  //            + size of one metadata entry
  //              * number of metadata entries)
  uint64_t offset = superblock_length + set_id * (1 + cache_associativity/4); // in sectors
  offset *= ssd_sector_size; // now offset is in bytes
  int ret = 0;
  lseek(ssd_fd, offset, SEEK_SET);
  if (reset) {
    std::shared_ptr<cache_metadata_set>set(new cache_metadata_set(set_id));
    write(ssd_fd, reinterpret_cast<char*>(set.get()), sizeof(cache_metadata_set));
    ret = reset_metadata_entries(offset+ 1*ssd_sector_size, set);
  }
  else {
    // TODO partially update metadata
    write(ssd_fd, reinterpret_cast<char*>(md_set.get()), sizeof(cache_metadata_set));
  }
  return ret;
}

int reset_metadata_entries(uint32_t start, std::shared_ptr<cache_metadata_set> md_set) {
  if (reset) {
    lseek(ssd_fd, start, SEEK_SET);
    auto len = ssd_sector_size * md_set->cache_size/4; // in bytes
    //debug("len=" + std::to_string(len));
    char buf[len];
    std::fill(buf, buf+len, '\0');
    return write(ssd_fd, buf, len);
  }
  return 0;
}

int read_metadata_set(int set_id, std::shared_ptr<cache_metadata_set> md_set) {
  auto offset = superblock_length + set_id * (1 + cache_associativity/4); // in sectors
  offset *= ssd_sector_size; // now offset is in bytes
  lseek(ssd_fd, offset, SEEK_SET);
  int ret = 0;
  ret = read(ssd_fd, reinterpret_cast<char*>(md_set.get()), sizeof(cache_metadata_set));
  if (ret < 0)
    return ret;
  return 0;
}

int write_metadata_entry(std::shared_ptr<cache_metadata_entry> md_entry) {
  md_entry->print();
  uint32_t offset = md_entry->index;
  offset *= ssd_sector_size;
  offset /= 4;
  offset += ssd_sector_size * (superblock_length + md_entry->index/cache_associativity + 1);
  lseek(ssd_fd, offset, SEEK_SET);
  int ret = 0;
  ret = write(ssd_fd, reinterpret_cast<char*>(md_entry.get()), sizeof(cache_metadata_entry));
  if (ret < 0)
    return ret;
  return 0;
}

int read_metadata_entry(uint32_t index, std::shared_ptr<cache_metadata_entry> md_entry) {
  auto offset = index;
  offset *= ssd_sector_size;
  offset /= 4;
  offset += ssd_sector_size * (superblock_length + index/cache_associativity + 1);
  lseek(ssd_fd, offset, SEEK_SET);
  int ret = 0;
  ret = read(ssd_fd, reinterpret_cast<char*>(md_entry.get()), sizeof(cache_metadata_entry));
  if (ret < 0)
    return ret;
  return 0;
}
