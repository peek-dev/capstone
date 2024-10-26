#!/usr/bin/env python3

from distutils.core import setup, Extension

module = Extension("rpi5_uartboost", sources = ["rpi5_uartboost.c"])

setup(name='PackageName', version='0.10', description='Booster module to interact with file-mapped UART on Raspberry Pi OS.', ext_modules = [module])
