#!/bin/bash

PKG_ROOT=$PWD
BOOST_VERSION="1_69_0"
BOOST_NAME=boost_$BOOST_VERSION
BOOST_PATH=$PKG_ROOT/boost
BOOST_TARBALL=$BOOST_PATH/"$BOOST_NAME".tar.bz2

echo -n "Checking if boost exists... "
if [ -d $BOOST_PATH/$BOOST_NAME/bin.v2 ] && [ -d $PKG_ROOT/include/boost ]; then
  echo "yes"
else
  echo "no"
  mkdir -p $BOOST_PATH
  if [ ! -d $BOOST_PATH/$BOOST_NAME ]; then
    echo -n "Checking if boost tarball is present... "
    if [ -f $BOOST_TARBALL ]; then
      echo "yes"
    else
      echo "no"
      wget -P $BOOST_PATH https://dl.bintray.com/boostorg/release/1.69.0/source/boost_1_69_0.tar.bz2
    fi

    echo -n "Checking if SHA256 of boost tarball matches... "
    TARGET_SHA256="8f32d4617390d1c2d16f26a27ab60d97807b35440d45891fa340fc2648b04406"
    if [ $(sha256sum $BOOST_TARBALL | awk '{ print $1; }') == $TARGET_SHA256 ]; then
      echo "yes"
    else
      echo "no"
      rm $BOOST_TARBALL
      wget -P $BOOST_PATH https://dl.bintray.com/boostorg/release/1.69.0/source/boost_1_69_0.tar.bz2
    fi

    echo -n "Decompressing tarball... "
    tar xjf $BOOST_TARBALL -C $BOOST_PATH
    echo "ok"
  fi

  cd $BOOST_PATH/$BOOST_NAME
  ./bootstrap.sh
  ./b2 install --with-iostreams --prefix="$PKG_ROOT"
fi
cd $PKG_ROOT
echo "Done."
