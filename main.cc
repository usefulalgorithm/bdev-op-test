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
  cerr << "Usage: " << pname << endl;
  exit(EXIT_FAILURE);
}

int main(int argc, char* argv[]) {
  string pname = argv[0];
  // default ssd location
  int opt, ssd_fd;
  int ssd_dev_size;
  int ssd_sector_size;
  while ((opt = getopt(argc, argv, "s:")) != -1) {
    switch (opt) {
      case 's':
        ssd_devname = optarg;
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
  cout << ssd_dev_size << endl;
  cout << ssd_sector_size << endl;
  /*
  cout << ssd_devname << ": device size = " << ssd_dev_size << ", sector size = " << ssd_sector_size << endl;
  */
  return 0;
}
