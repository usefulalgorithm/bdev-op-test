#include "common.h"
string   pname                = ""; // program name
string   ssd_devname          = "/dev/sdf2";     // device name
uint64_t base_size            = 1;
string   base_size_str        = "B";          // defaults to Bytes
uint64_t ssd_dev_size         = 0;
uint32_t ssd_sector_size      = 0;
uint32_t ssd_block_size       = 8;           // defaults to 4K
uint32_t object_size          = 8*1024;  // defaults to 4M
uint64_t cache_entries        = 0;
uint64_t cache_associativity  = 0;
uint16_t cache_set_count      = 0;
