#!/bin/bash

PKG_ROOT=$PWD
TARGETS="iostreams"
BOOST_VERSION_NUM="1.69.0"
BOOST_VERSION_STR="1_69_0"
BOOST_NAME=boost_$BOOST_VERSION_STR
BOOST_PATH=$PKG_ROOT/boost
BOOST_TARBALL=$BOOST_PATH/"$BOOST_NAME".tar.bz2
BOOST_TARGETS=""

check_target() {
  target=$1
  if [ ! -f $PKG_ROOT/lib/libboost_"$target".a ] || [ ! -f $PKG_ROOT/lib/libboost_"$target".so ]; then
    echo "false"
  else
    echo "true"
  fi
}

check_all_targets() {
  for target in $TARGETS; do
    if [ $(check_target $target) == "false" ]; then
      echo "false"
      return 0
    fi
  done
  echo "true"
}

echo -n "Checking if boost is built... "
if [ -d $BOOST_PATH/$BOOST_NAME/bin.v2 ] && [ -d $PKG_ROOT/include/boost ] && [ $(check_all_targets) == "true" ]; then
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
      wget -P $BOOST_PATH https://dl.bintray.com/boostorg/release/$BOOST_VERSION_NUM/source/"$BOOST_VERSION_STR".tar.bz2
    fi

    echo -n "Checking if SHA256 of boost tarball matches... "
    TARGET_SHA256="8f32d4617390d1c2d16f26a27ab60d97807b35440d45891fa340fc2648b04406"
    if [ $(sha256sum $BOOST_TARBALL | awk '{ print $1; }') == $TARGET_SHA256 ]; then
      echo "yes"
    else
      echo "no"
      rm $BOOST_TARBALL
      wget -P $BOOST_PATH https://dl.bintray.com/boostorg/release/$BOOST_VERSION_NUM/source/"$BOOST_VERSION_STR".tar.bz2
    fi

    echo -n "Decompressing tarball... "
    tar xjf $BOOST_TARBALL -C $BOOST_PATH
    echo "ok"
  fi

  cd $BOOST_PATH/$BOOST_NAME
  ./bootstrap.sh
  for target in $TARGETS; do
    BOOST_TARGETS=$BOOST_TARGETS" --with-$target"
  done
  ./b2 install -d0 $BOOST_TARGETS --prefix="$PKG_ROOT"
fi
cd $PKG_ROOT
echo "Done."
