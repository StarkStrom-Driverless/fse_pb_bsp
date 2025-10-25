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

python3 -m venv .venv

source .venv/bin/activate

pip install can
pip install click
pip install cryptography
pip install intelhex
pip install cbor
pip install pyyaml