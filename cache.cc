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
  log << "device_name: " << device_name
    << ", sector_size=" << sector_size << "B"
    << ", device_size=" << device_size << "sec."
    << ", block_size=" << block_size << "sec."
    << ", object_size=" << object_size << "sec."
    << ", entries=" << entries
    << ", associativity=" << associativity
    << ", sets=" << sets
    << ", md_len=" << md_len << "sec."
    << ", data_len=" << data_len << "sec." << endl;
}

cache_metadata_set::cache_metadata_set(cache_metadata_set* _cms) {
  set_id = _cms->set_id;
  lru_head = _cms->lru_head;
  lru_tail = _cms->lru_tail;
  invalid_head = _cms->invalid_head;
  PBA_begin = _cms->PBA_begin;
  PBA_end = _cms->PBA_end;
  checksum = _cms->checksum;
  unclean = _cms->unclean;
}

cache_metadata_set::cache_metadata_set(int _set_id) {
  set_id = _set_id;
  lru_head = CACHE_NULL, lru_tail = CACHE_NULL;
  lru_size = 0;
  invalid_head = CACHE_NULL;
  auto _PBA_begin = 1 + metadata_length + (_set_id) * cache_obj_size * cache_associativity;
  PBA_begin = _PBA_begin;
  PBA_end = _PBA_begin + cache_obj_size * cache_associativity;
  PBA_begin *= ssd_sector_size;
  PBA_end *= ssd_sector_size;
  checksum = 0;
  unclean = false;
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
  auto check_if_null = [] (uint32_t x) {
    if (x == 0xFFFFFFFF)
      return std::string("NULL");
    else
      return std::to_string(x);
  };
  log << "set_id=" << int(set_id)
    << ", lru_head=" << std::hex << check_if_null(lru_head)
    << ", lru_tail=" << check_if_null(lru_tail)
    << ", invalid_head=" << check_if_null(invalid_head)
    << ", PBA_begin=" << PBA_begin << "B"
    << ", PBA_end=" << PBA_end << "B"
    << ", checksum=" << std::dec << checksum
    << ", unclean=" << unclean << endl;
}

void cache_metadata_entry::print() {
  log << "image_id=" << image_id
    << ", object_id=" << object_id
    << ", index=" << index
    << ", valid_bit=" << valid_bit
    << ", PBA=" << PBA << "B"
    << ", lru_prev=" << lru_prev
    << ", lru_next=" << lru_next
    << ", prev=" << prev
    << ", next=" << next << endl;
}

int write_superblock(int fd, char* buf, size_t len) {
  // TODO are there other ways to do this?
  return write(fd, buf, len);
}

int read_superblock(int fd, char* buf) {
  // TODO are there other ways to do this?
  return read(fd, buf, 512);
}

int write_metadata_set(int fd, int set_id) {
  // get metadata set offset
  //
  // cache_associativity = length of entries in a set
  // length of one entry = 1 sector
  // length of one metadata set info = 1 sector
  //
  // offset = (length of superblock)
  //        + (which set is this)
  //          * (length of one metadata set info
  //            + length of one metadata entry)
  uint64_t offset = superblock_length + set_id * (1 + cache_associativity); // in sectors
  offset *= ssd_sector_size; // now offset is in bytes
  auto md_set = new cache_metadata_set(set_id);
#ifdef DEBUG
  log << "offset=" << offset << "B" << endl;
  md_set->print();
#endif
  int ret = 0;
  if (reset) {
    lseek(fd, offset, SEEK_SET);
    write(fd, (char*)md_set, ssd_sector_size);
    ret = reset_metadata_entries(fd, offset+ 1*ssd_sector_size);
  }
  else {
    // TODO partially update metadata
  }
  delete md_set;
  return ret;
}

int reset_metadata_entries(int fd, uint32_t start) {
  if (reset) {
    lseek(fd, start, SEEK_SET);
    auto len = ssd_sector_size * cache_associativity; // in bytes
    char buf[len];
    std::fill(buf, buf+len, '\0');
#ifdef DEBUG
    log << start << "~" << start + len<< endl;
#endif
    return write(fd, buf, len);
  }
  return 0;
}

#if 1
int read_metadata_set(int fd, int set_id, cache_metadata_set* md_set) {
  auto offset = superblock_length + set_id * (1 + cache_associativity); // in sectors
  offset *= ssd_sector_size; // now offset is in bytes
  lseek(fd, offset, SEEK_SET);
  int ret = 0;
  ret = read(fd, reinterpret_cast<char*>(md_set), sizeof(cache_metadata_set));
  if (ret < 0)
    return ret;
  return 0;
}
#else
int read_metadata_set(int fd, int set_id, cache_metadata_set*& md_set) {
  auto offset = superblock_length + set_id * (1 + cache_associativity); // in sectors
  offset *= ssd_sector_size; // now offset is in bytes
  lseek(fd, offset, SEEK_SET);
  log << offset << "~" << offset+ssd_sector_size << endl;
  auto len = ssd_sector_size * metadata_set_info_length;

  char md_set_buf[len];
  int ret = read(fd, md_set_buf, len);
  if (ret < 0)
    return ret;
  md_set = new cache_metadata_set((cache_metadata_set*)md_set_buf);

#ifdef DEBUG
  log << "read " << ret << " bytes" << endl;
  md_set->print();
#endif
  return 0;
}
#endif
