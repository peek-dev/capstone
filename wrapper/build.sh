#!/usr/bin/bash

./setup.py build
mv ./build/lib.linux-aarch64-cpython-311/rpi5_uartboost*.so .
rm -r ./build/