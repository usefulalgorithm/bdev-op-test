# bdev-op-test

Author: [Andy Lii](mailto:usefulalgorithm@gmail.com)

## Description

The purpose of this project is to investigate ways to directly perform read /
write operations on block devices, without using filesystem to manage metadata.

Currently I'm operating on a CentOS in a Docker container.

In this project I use [SpookyHash](http://burtleburtle.net/bob/hash/spooky.html)
and [stx-btree](https://github.com/bingmann/stx-btree) projects as I do not have
the prowess to implement all of these functionalities. 

SpookyHash is a 128-bit noncryptographic hash written by Bob Jenkins and placed 
in the public domain under [CC0](https://creativecommons.org/publicdomain/zero/1.0/).
SpookyHash is chosen for its lightweight implementation and nice performance.

The STX B+ Tree package is a set of C++ template classes implementing a B+ tree
key/data container in main memory. It is written by Timo Bingmann, and released 
under the Boost Software License, Version 1.0.

## Install

```
$ ./check_boost.sh && make
```
