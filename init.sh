#!/bin/bash

cp test/main.c ..
cp test/Makefile ..
cp test/.gitignore ..
cp test/FreeRTOSConfig.h ../usr/inc
 
mkdir -p ../usr/src
mkdir -p ../usr/inc

git submodule init
git submodule update
cd fse_pb_bsp
git submodule init
git submodule update
cd libopencm3
make
cd ../..
