#!/usr/bin/env python3

from distutils.core import setup, Extension

module = Extension("rpi_uartboost", sources = ["rpi_uartboost.c"])

setup(name='PackageName', version='0.40', description='Booster module to interact with file-mapped UART on Raspberry Pi OS.', ext_modules = [module])
