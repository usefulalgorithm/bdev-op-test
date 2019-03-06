# bdev-op-test [![Build Status](https://travis-ci.com/usefulalgorithm/bdev-op-test.svg?branch=master)](https://travis-ci.com/usefulalgorithm/bdev-op-test)

Author: [Andy Lii](mailto:usefulalgorithm@gmail.com)

## Description

The purpose of this project is to investigate ways to directly perform read /
write operations on block devices, without using filesystem to manage metadata.

Currently I'm operating on a CentOS in a Docker container.

In this project I use [SpookyHash](http://burtleburtle.net/bob/hash/spooky.html) 
to create hashed indices for pool, image and object names. SpookyHash is a 
128-bit noncryptographic hash written by Bob Jenkins and placed in the public 
domain under [CC0](https://creativecommons.org/publicdomain/zero/1.0/). 
SpookyHash is chosen for its lightweight implementation and nice performance.

## Install


_Tip: build with `make debug` for much more illustrative outputs._

```
$ ./check_boost.sh && make
```

## Usage

__IMPORTANT: Remember to change the SSD location!!!__

In my use case, each cache object has a size of 4 MB. This value is defined as a 
fixed static variable in this project, but it shouldn't be too hard for you to change
this value. ;)

```
Usage: ./bdev-op-test [-s SSD_LOCATION] [-K | -M | -G] [-n CACHE_ENTRIES] [-a ASSOCIATIVITY] [-r] [-W] [-h | -?] <-p OBJ | -g OBJ>
Options:
        -s              sets location of the SSD. If unspecified, the default value is /dev/sdf4
        -K | -M | -G    sets base for displaying volume size
        -n              sets number of cache entries
        -a              sets cache associativity, fully associative if not set
        -r              resets the cache
        -W              wipes out the cache superblock
        -h, -?          prints this help message
        -p OBJ          puts OBJ into the cache
        -g OBJ          gets OBJ from the cache
```
