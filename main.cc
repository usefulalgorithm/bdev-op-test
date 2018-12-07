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

#include "SpookyV2.h"
#include "stx/btree_map.h"

using std::string;
using std::cout;
using std::cerr;
using std::endl;

const int buf_size = 512;
char buffer[buf_size];
string ssd_devname("/dev/sdf2");

#define whine (cerr << pname << ": ")
#define DEV_PATHLEN   128
#define NOOP          0
#define PUT           1
#define GET           2
#define EVICT         3

#define SEED          0xbabeface

typedef uint32_t hash_t;

void usage(string pname) {
  cerr << "Usage: " << pname << " [-s SSD_LOCATION] [-K | -M | -G] [-n CACHE_ENTRIES] [-r] [-W] [-h | -?] <-p OBJ | -g OBJ | -e OBJ>" << endl; 
  cerr << "Options:" << endl;
  cerr << "\t-s\t\tsets location of the SSD. If unspecified, the default value is " << ssd_devname << endl;
  cerr << "\t-K | -M | -G\tsets base for displaying volume size" << endl;
  cerr << "\t-n\t\tsets number of cache entries" << endl;
  cerr << "\t-r\t\tresets the cache" << endl;
  cerr << "\t-W\t\twipes out the cache superblock" << endl;
  cerr << "\t-h, -?\t\tprints this help message" << endl;
  cerr << "\t-p OBJ\t\tputs OBJ into the cache" << endl;
  cerr << "\t-g OBJ\t\tgets OBJ from the cache" << endl;
  cerr << "\t-e OBJ\t\tevicts OBJ from the cache" << endl;
  exit(EXIT_FAILURE);
}

int write_superblock(int fd, char* buf, size_t len) {
  // TODO are there other ways to do this?
  return write(fd, buf, len);
}

int read_superblock(int fd, char* buf) {
  // TODO are there other ways to do this?
  return read(fd, buf, 512);
}

// cache structure:
//
//      [ superblock | b+ tree | data ] 

struct superblock {
  char device_name[DEV_PATHLEN];
  uint32_t sector_size; // how many bytes per sector
  uint64_t device_size; // in sectors
  uint32_t block_size; // in sectors
  uint32_t md_block_size; // in sectors
  uint32_t object_size; // in sectors
  uint64_t entries;
  uint64_t tree_size; // in sectors
  void print() {
    cout << "device_name: " << device_name
      << ", sector_size=" << sector_size
      << ", device_size=" << device_size
      << ", block_size=" << block_size
      << ", md_block_size=" << md_block_size
      << ", object_size=" << object_size
      << ", entries=" << entries 
      << ", tree_size=" << tree_size << endl;
  }
};

int main(int argc, char* argv[]) {
  string pname = argv[0];
  // default ssd location
  int opt, ssd_fd;
  uint64_t base_size = 1;
  string base_size_str = "B";
  uint64_t ssd_dev_size = 0;
  uint32_t ssd_sector_size = 0;
  uint32_t ssd_block_size = 8;
  uint32_t ssd_md_block_size = 8;
  uint32_t object_size = 1024*4096/512;
  uint64_t cache_entries = 0;
  uint64_t cache_tree_size = 0;
  bool reset = false;
  bool wipe_superblock = false;
  uint8_t operation = NOOP;
  string object_name;
  while ((opt = getopt(argc, argv, "s:GMKn:rWh?p:g:")) != -1) {
    switch (opt) {
      case 's':
        ssd_devname = optarg;
        break;
      case 'r':
        reset = true;
        break;
      case 'p':
        if (operation != NOOP) {
          cerr << "Can only do one operation at a time" << endl;
          usage(pname);
        }
        operation = PUT;
        object_name = optarg;
        break;
      case 'g':
        if (operation != NOOP) {
          cerr << "Can only do one operation at a time" << endl;
          usage(pname);
        }
        operation = GET;
        object_name = optarg;
        break;
      case 'e':
        if (operation != NOOP) {
          cerr << "Can only do one operation at a time" << endl;
          usage(pname);
        }
        operation = EVICT;
        object_name = optarg;
        break;
      case 'W':
        wipe_superblock = true;
        break;
      case 'G':
        base_size_str = "GB";
        base_size = 1 << 30;
        break;
      case 'M':
        base_size_str = "MB";
        base_size = 1 << 20;
        break;
      case 'K':
        base_size_str = "KB";
        base_size = 1 << 10;
        break;
      case 'n':
        try {
          cache_entries = std::stoull(optarg);
        } catch (const std::exception& e) {
          whine << "Invalid number of cache entries." << endl;
          usage(pname);
        }
        break;
      case '?':
      case 'h':
      default:
        usage(pname);
    }
  }
  ssd_fd = open(ssd_devname.c_str(), O_RDWR);
  if (ssd_fd < 0) {
    whine << "Cannot open " << ssd_devname << ": " << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  }
  lseek(ssd_fd, 0, SEEK_SET);

  // wipe everything!
  if (wipe_superblock) {
    std::fill(buffer, buffer+512, '\0');
    if (write(ssd_fd, buffer, 512) < 0) {
      whine << "Cannot wipe out ssd superblock " << ssd_devname << ": " << strerror(errno) << endl;
      exit(EXIT_FAILURE);
    }
#ifdef DEBUG
    cout << "Wiped entire superblock!" << endl;
#endif
    if (close(ssd_fd) < 0) {
      whine << "Cannot close " << ssd_devname << ": " << strerror(errno) << endl;
      exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
  }

  // get ssd and cache infos, then overwrite ssd superblock
  if (reset) {
    if (ioctl(ssd_fd, BLKGETSIZE, &ssd_dev_size) < 0) {
      whine << "Cannot get SSD device size: " << strerror(errno) << endl;
      exit(EXIT_FAILURE);
    }
    if (ioctl(ssd_fd, BLKSSZGET, &ssd_sector_size) < 0) {
      whine << "Cannot get SSD sector size: " << strerror(errno) << endl;
      exit(EXIT_FAILURE);
    }
    if (object_size % ssd_sector_size) {
      whine << "Object size " << object_size << " should be a multiple of sector size " << ssd_sector_size << endl;
      exit(EXIT_FAILURE);
    }
    if (!cache_entries) {
      whine << "Invalid number of cache entries." << endl;
      usage(pname);
    }
#ifdef DEBUG
    cout << ssd_devname << ": device size = " << (ssd_dev_size * 512)/base_size << base_size_str
      << ", sector size = " << ssd_sector_size << "B"
      << ", block size = " << ssd_block_size*512 << "B"
      << ", object size = " << object_size*512/base_size << base_size_str << endl;
#endif
    uint64_t max_cache_entries = std::min((ssd_dev_size * 512) / object_size, uint64_t((1UL << 32) -1));
    if (cache_entries > max_cache_entries) {
      whine << "Maximum entry number exceeded. The limit is " << max_cache_entries << endl;
      exit(EXIT_FAILURE);
    }

    cache_tree_size = sizeof(hash_t) * cache_entries / 512 + 1;
    
    // set the attributes in the superblock
    struct superblock* header = (struct superblock*)malloc(sizeof(struct superblock));
    memset(header, 0, sizeof(struct superblock));
    strncpy(header->device_name, ssd_devname.c_str(), DEV_PATHLEN);
    header->sector_size = ssd_sector_size;
    header->device_size = ssd_dev_size;
    header->block_size = ssd_block_size;
    header->md_block_size = ssd_md_block_size;
    header->object_size = object_size;
    header->entries = cache_entries;
    header->tree_size = cache_tree_size;
    if (write_superblock(ssd_fd, (char*)header, sizeof(struct superblock)) < 0) {
      whine << "Cannot reset ssd superblock " << ssd_devname << ": " << strerror(errno) << endl;
      exit(EXIT_FAILURE);
    }
    free(header);
  }

  // read existing superblock
  else {
    if (read_superblock(ssd_fd, buffer) < 0) {
      whine << "Cannot read SSD superblock " << ssd_devname << ": " << strerror(errno) << endl;
      exit(EXIT_FAILURE);
    }
    struct superblock* header = (struct superblock*)buffer;
#ifdef DEBUG
    header->print();
#endif
    if (!strlen(header->device_name)) {
      whine << "No ssd device given. Please reset cache superblock first." << endl;
      exit(EXIT_FAILURE);
    }
    // we're gonna put object into cache
    if (operation == PUT) {
      // check if object exists
      if (!std::experimental::filesystem::exists(object_name)) {
        whine << object_name << " does not exist" << endl;
        exit(EXIT_FAILURE);
      }
      uint32_t hashed = SpookyHash::Hash32(object_name.c_str(), object_name.length(), SEED);
#ifdef DEBUG
      cout << "hashed=" << hashed << endl;
#endif
    }
    // we're gonna get object from cache
    else if (operation == GET) {
      uint32_t hashed = SpookyHash::Hash32(object_name.c_str(), object_name.length(), SEED);
#ifdef DEBUG
      cout << "hashed=" << hashed << endl;
#endif
    }
    // we're gonna evict object from cache
    else {
      hash_t hashed = SpookyHash::Hash32(object_name.c_str(), object_name.length(), SEED);
#ifdef DEBUG
      cout << "hashed=" << hashed << endl;
#endif
    }
  }

  if (close(ssd_fd) < 0) {
    whine << "Cannot close " << ssd_devname << ": " << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  }
  return EXIT_SUCCESS;
}
