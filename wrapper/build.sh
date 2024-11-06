#!/usr/bin/bash

./setup.py build
mv build/lib.linux-aarch64-3.11/rpi_uartboost*.so .
rm -r ./build/

