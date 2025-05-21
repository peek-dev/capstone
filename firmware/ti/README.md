This should contain a directory (or symlink) that contains a compliant Texas
Instruments driverlib installation to support the firmware. Such an installation
may be obtained from TI's MSPM0 SDK (target: LP-MSPM0G3507) or similar repositories.

A compliant installation includes, but need not be limited to:
* `dl_uart` source files and headers, including for `dl_uart_main` and
  `dl_uart_extend`
* `dl_spi` source files and headers
* `dl_adc12` source files and headers
* `dl_timerg` source files and headers
* SYSCTL, timer, and common headers

Note that most of this code will be released under TI licenses, NOT under the
GNU GPLv3.

For more information and a complete list of dependencies, please see the code
in `/firmware/src/`.
