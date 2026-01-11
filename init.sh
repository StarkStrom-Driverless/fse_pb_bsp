#!/bin/bash

mkdir -p ../usr/src
mkdir -p ../usr/inc

cp -n test/main.c ..
cp -n test/Makefile ..
cp -n test/.gitignore ..
cp -n test/FreeRTOSConfig.h ../usr/inc
 

cd libopencm3
make
cd ../..


ln -s fse_pb_bsp/tools/ss ss

python3 -m venv .venv

source .venv/bin/activate

pip install --upgrade pip

pip install can
pip install click
pip install cryptography
pip install intelhex
pip install cbor
pip install pyyaml
pip install python-can
pip install telnetlib3