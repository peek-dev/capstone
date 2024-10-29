#!/usr/bin/bash

./setup.py build
mv ./build/lib.linux-aarch64-cpython-311/rpi_uartboost*.so .
rm -r ./build/
