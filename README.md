BTLE
====

This is a fork of floe's BTLE library; it has some improvements to make it able
to mimick nRF8001 and nRF51822 beacon packets and to work with maintained RF24.
More info on http://simonebaracchi.eu/posts/temperature-beacon/ .

Arduino library for basic Bluetooth Low Energy support using the nRF24L01+
(basic support = sending & receiving on the advertising broadcast channel)

You will also need to install the RF24 library installable either from the Arduino IDE or https://github.com/TMRh20/RF24 .

Note: the BTLE class and the examples are licensed under GPLv3. However, the
helper functions in btle.inc are (C) 2012 by Dmitry Grinberg under a separate
license (see file for details).
