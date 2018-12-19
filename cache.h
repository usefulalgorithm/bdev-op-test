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
