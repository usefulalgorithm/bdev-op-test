# bdev-op-test

The purpose of this project is to investigate ways to directly perform read /
write operations on block devices, without using filesystem to manage metadata.

Currently I'm operating on a CentOS in a Docker container.

In this project I use [SpookyHash](http://burtleburtle.net/bob/hash/spooky.html),
a 128-bit noncryptographic hash written by Bob Jenkins and placed in the public 
domain under [CC0](https://creativecommons.org/publicdomain/zero/1.0/). SpookyHash
is chosen for its lightweight implementation and nice performance.

Author: [Andy Lii](mailto:usefulalgorithm@gmail.com)
