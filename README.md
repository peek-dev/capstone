# Capstone: Square Dance

This is the code for the microcontroller of a smart chessboard project (the
C.H.E.S.S.B.O.A.R.D., or Chess Helper, Evaluator, and Study Supporter to Boost
Observation, Acumen, Reasoning, and Deduction) at the University of Virginia.
At some point, this document will be updated to explain the project and link to
our theses. At the moment, things are volatile and we have not written our
theses.

This project represents the combined efforts of John E. Berberian, Jr., and
Paul D. Karhnak on behalf of the "Square Dance" team. The authors thank you for
your interest in this project.

# Code Structure

The `firmware/` directory holds all project code, including project
organization and device configuration files, for the target microcontroller.
This software does NOT include the Texas Instruments Driver Library
("driverlib"); rather, there is a "stub" directory with a README that marks the
location where the driverlib should be separately installed in the project
structure.

The `wrapper/` directory holds all project code for the target Raspberry Pi
system, and the `rpi-admin/` directory holds system administration scripts to
create and activate the necessary environment. With the exception of creating
and activating the Python virtual environment (venv) where `chess` and `serial`
are installed, and where the wrapper scripts (`sf_wrapper.py` and
`wrapper_util.py`) run, most tasks that the scripts in `rpi-admin/` execute are
optional accelerations.

# Dependencies

The firmware was originally deployed on a Texas Instruments LP-MSPM0G3507. The
firmware is generally appropriate for an ARM Cortex-M0+ based microcontroller
assuming that the MCU has:
* At least one hardware ADC module
* At least two hardware SPI modules
* At least one hardware UART module
* Support for at least _X_ GPIO pins

The development environment used for the firmware was the [Texas Instruments
Code Composer Studio Theia IDE, version
1.5.0](https://www.ti.com/tool/MSPM0-SD://www.ti.com/tool/download/CCSTUDIO-THEIA/1.5.0).
Any version of Theia >= 1.5.0 should be appropriate to build and flash the
firmware onto the target MCU. Note the difference between Theia and TI's legacy
Code Composer Studio tool; Theia should be used, NOT legacy CCS.

A conforming Texas Instruments driverlib installation is required since the
firmware frequently makes calls to TI routines; such an installation may be
procured from [TI's MSPM0 SDK](https://www.ti.com/tool/MSPM0-SDK) or a similar
source.

The Python code was written with Python 3.11 and runs on Linux (Raspberry Pi OS
Bookworm). The code specifically targets the Raspberry Pi 4B, which maps a
general-purpose UART peripheral to the `/dev/ttyS0` device that `/dev/serial0`
symlinks to. On the Raspberry Pi 5, `/dev/serial0` instead symlinks to the
dedicated debug UART port, which is NOT suitable for this application. If this
code is deployed on a Raspberry Pi 5, the `serial.Serial` constructor call in
`sf_wrapper.py` should be changed to target the path `/dev/ttyAMA0` instead of
`/dev/serial0` (note that this assumes the UART overlay has been loaded on the
Raspberry Pi 5).

# Licensing

Except as otherwise noted, this software is released under the GNU General
Public License version 3.0. A general notice is published below and in most
source and header files contained in this repository:

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

Note that most software included from Texas Instruments (TI) to support the
MSPM0G3507's operation, as well as the FreeRTOS core software, is NOT released
under the GPLv3.0. FreeRTOS is distributed under the MIT License, while TI
distributes its driver libraries and similar software for devices like the
MSPM0G3507 under its own licensing scheme. The GNU GPLv3.0 terms DO NOT apply
to these included software modules and the authors of this repository (John E.
Berberian, Jr., and Paul D. Karhnak) do not make any attempt to assert
additional terms.

