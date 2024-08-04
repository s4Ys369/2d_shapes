#!/bin/bash
set -e

if [ "$1" = "clean" ] ; then
  echo "Cleaning up..."
  make -C c clean
  make -C cpp clean
  exit 0
fi

if [ "$1" = "clean_c" ] ; then
  echo "Cleaning C..."
  make -C c clean
  exit 0
fi

if [ "$1" = "clean_cpp" ] ; then
  echo "Cleaning C++..."
  make -C cpp clean
  exit 0
fi

if [ "$1" = "c" ] ; then
  echo "Building C..."
  make -C c -j4
  exit 0
fi

if [ "$1" = "cpp" ] ; then
  echo "Building C++..."
  make -C c -j4
  exit 0
fi

echo "Building..."
make -C c -j4
make -C cpp -j4

echo "Build done!"