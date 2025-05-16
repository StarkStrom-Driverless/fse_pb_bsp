#!/bin/bash

mv test/main.c ..
mv test/Makefile ..
mkdir ../usr/src
mkdir ../usr/inc

git submodule init
git submodule update
cd fse_pb_bsp
git submodule init
git submodule update
cd libopencm3
make
cd ../..
