#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include "fcntl.h"
#include "unistd.h"
#include <sys/ioctl.h>
#include <linux/fs.h>

using std::string;
using std::cout;
using std::cerr;
using std::endl;

char buffer[512];
string ssd_devname("/dev/sdf2");

void usage(string pname) {
  cerr << "Usage: " << pname << " [-s SSD_LOCATION] [-K | -M | -G]" << endl;
  cerr << "Options:" << endl;
  cerr << "\t-s\t\tlocation of the SSD. If unspecified, the default value is " << ssd_devname << endl;
  cerr << "\t-K | -M | -G\tset base for displaying volume size" << endl;
  exit(EXIT_FAILURE);
}

int main(int argc, char* argv[]) {
  string pname = argv[0];
  // default ssd location
  int opt, ssd_fd;
  uint64_t base_size = 1;
  string base_size_str = "B";
  uint64_t ssd_dev_size;
  uint64_t ssd_sector_size;
  uint64_t object_size = 1024*4096;
  while ((opt = getopt(argc, argv, "s:GMK")) != -1) {
    switch (opt) {
      case 's':
        ssd_devname = optarg;
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
      default:
        usage(pname);
    }
  }
  ssd_fd = open(ssd_devname.c_str(), O_RDONLY);
  if (ssd_fd < 0) {
    cerr << "Cannot open " << ssd_devname << ": " << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  }
  lseek(ssd_fd, 0, SEEK_SET);
  if (read(ssd_fd, buffer, 512) < 0) {
    cerr << "Cannot read SSD superblock " << ssd_devname << ": " << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  }
  if (ioctl(ssd_fd, BLKGETSIZE, &ssd_dev_size) < 0) {
    cerr << "Cannot get SSD device size: " << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  }
  if (ioctl(ssd_fd, BLKSSZGET, &ssd_sector_size) < 0) {
    cerr << "Cannot get SSD sector size: " << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  }
  if (object_size % ssd_sector_size) {
    cerr << "Object size " << object_size << " should be a multiple of sector size " << ssd_sector_size << endl;
    exit(EXIT_FAILURE);
  }
#ifdef DEBUG
  cout << ssd_devname << ": device size = " << (ssd_dev_size * 512)/base_size << base_size_str
    << ", sector size = " << ssd_sector_size << endl;
  cout << ssd_devname << ": " << ssd_dev_size << " sectors" << endl;
#endif
  return 0;
}
