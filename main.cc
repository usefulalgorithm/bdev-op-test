#include <iostream>
#include <algorithm>
#include <string>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <limits>
#include <exception>
#include "fcntl.h"
#include "unistd.h"
#include <sys/ioctl.h>
#include <linux/fs.h>

using std::string;
using std::cout;
using std::cerr;
using std::endl;

const int buf_size = 512;
char buffer[buf_size];
string ssd_devname("/dev/sdf2");

#define whine (cerr << pname << ": ")

void usage(string pname) {
  cerr << "Usage: " << pname << " [-s SSD_LOCATION] [-K | -M | -G] [-n CACHE_ENTRIES] [-r]" << endl; 
  cerr << "Options:" << endl;
  cerr << "\t-s\t\tlocation of the SSD. If unspecified, the default value is " << ssd_devname << endl;
  cerr << "\t-K | -M | -G\tset base for displaying volume size" << endl;
  cerr << "\t-n\t\tnumber of cache entries" << endl;
  cerr << "\t-r\t\tresets the cache" << endl;
  exit(EXIT_FAILURE);
}

int main(int argc, char* argv[]) {
  string pname = argv[0];
  // default ssd location
  int opt, ssd_fd;
  uint64_t base_size = 1;
  string base_size_str = "B";
  uint64_t ssd_dev_size = 0;
  uint64_t ssd_sector_size = 0;
  uint64_t object_size = 1024*4096;
  uint64_t cache_entries = 0;
  bool reset = false;
  while ((opt = getopt(argc, argv, "s:GMKn:r")) != -1) {
    switch (opt) {
      case 's':
        ssd_devname = optarg;
        break;
      case 'r':
        reset = true;
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
  // FIXME use buf_size instead of 512
  if (reset) {
    std::fill(buffer, buffer+512, 0);
    if (write(ssd_fd, buffer, 512) < 0) {
      whine << "Cannot reset cache metadata " << ssd_devname << ": " << strerror(errno) << endl;
      exit(EXIT_FAILURE);
    }
  }
  // TODO read object_size, cache_entries and so on from the superblock
  if (read(ssd_fd, buffer, 512) < 0) {
    whine << "Cannot read SSD superblock " << ssd_devname << ": " << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  }
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
  // TODO read metadata from buffer
  if (!cache_entries) {
    whine << "Invalid number of cache entries." << endl;
    usage(pname);
  }
#ifdef DEBUG
  cout << ssd_devname << ": device size = " << (ssd_dev_size * 512)/base_size << base_size_str
    << ", sector size = " << ssd_sector_size << "B"
    << ", object size = " << object_size/base_size << base_size_str << endl;
#endif
  uint64_t max_cache_entries = std::min((ssd_dev_size * 512) / object_size, uint64_t((1UL << 32) -1));
  if (cache_entries > max_cache_entries) {
    whine << "Maximum entry number exceeded. The limit is " << max_cache_entries << endl;
    exit(EXIT_FAILURE);
  }
  if (close(ssd_fd) < 0) {
    whine << "Cannot close " << ssd_devname << ": " << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  }
  return 0;
}
