#ifndef UTILS_H
#define UTILS_H
#include "common.h"

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

#endif
