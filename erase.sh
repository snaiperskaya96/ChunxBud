#!/bin/bash

make flashinit # empty the controller
python2 /opt/esp-open-sdk-gcc8/xtensa-lx106-elf/bin/esptool.py -p /dev/ttyUSB0 -b 500000 write_flash -ff 40m -fm qio -fs detect 0x00000 out/Esp8266/debug/firmware/rboot.bin # flash rboot
python2 /opt/esp-open-sdk-gcc8/xtensa-lx106-elf/bin/esptool.py -p /dev/ttyUSB0 -b 115200 write_flash -ff 40m -fm qio -fs 4M 0x2000 out/Esp8266/debug/firmware/rom0.bin # flash rom0 on slot 0