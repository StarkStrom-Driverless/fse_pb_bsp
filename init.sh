#!/bin/bash

mkdir -p ../usr/src
mkdir -p ../usr/inc

cp test/main.c ..
cp test/Makefile ..
cp test/.gitignore ..
cp test/FreeRTOSConfig.h ../usr/inc
 

cd libopencm3
make
cd ../..


ln -s fse_pb_bsp/tools/ss ss

python -m venv .venv

pip install -r fse_pb_bsp/tools/requirements.txt

source .venv/bin/activate