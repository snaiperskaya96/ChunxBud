ChunxBud
===========

Introduction
------------

ChunxBud is Chunx's best buddy. Helps him staying hydrated.

Building
--------

Make sure you can build a Smeng project. https://github.com/SmingHub/Sming
We currently use https://github.com/someburner/esp-open-sdk.git instead of the esp-open-sdk offered with Sming as that doesn't support gcc8.

Flashing
--------

When you are done with the setup, running `make flash` is all you need.

Technical Notes
---------------

ChunxBud connects to a bunch of urls before doing its homeworks. This allows OTA updates for both production and development, and if none of those URLs are available, ChunxBud will go back to sleep. Check src/UpdateManager.cpp for more information.

ChunxBud is developed and tested on a esp8266 and esp12-f.
In order to power up a standalone board, follow this scheme with the only difference of linking RST and GPIO16 together (this allows deep sleep).

![ESP12 Power Scheme](docs/power-scheme.jpg?raw=true "ESP12 Power Scheme")

Special thanks
--------------

Thanks.