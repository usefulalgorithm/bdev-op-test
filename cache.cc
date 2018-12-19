#include "cache.h"

void superblock::print() {
  log << "device_name: " << device_name
    << ", sector_size=" << sector_size
    << ", device_size=" << device_size
    << ", block_size=" << block_size
    << ", md_block_size=" << md_block_size
    << ", object_size=" << object_size
    << ", entries=" << entries 
    << ", tree_size=" << tree_size << endl;
}
