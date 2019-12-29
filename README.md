# CustomP1UartComponent

This is my first custom component for EspHome. It can be used to read DSMR data from the P1 port of dutch smart meters

The work is based on these projects:
- https://esphome.io/custom/uart.html (The project we're the custom component is for)
- https://github.com/rspaargaren/DSMR_ESPHOME (We shared thoughts on how to read the P1 port)
- https://github.com/matthijskooijman/arduino-dsmr (The library that i use to parse the telegrams)
- https://github.com/brandond/esphome-tuya_pir (Example how to read data from UART)
- http://domoticx.com/p1-poort-slimme-meter-uitlezen-hardware/ (Information about hardware requirements)

## Hardware

I used a Wemos D1 mini to connect to the P1 port but it will probably work with most ESP boards. You need some kind of hardware inverter because the UART component doesn't support inverting the signal with a software setting.
I connected D2 to port A2 from a 7404 IC hardware inverter and Port A1 from the inverter to Port 5 from the P1 connector.
Port D5 from the Wemos is connected to port 2 from the p1 connector. This is used to request a message from the meter.
R1 is needed for my Iskra meter. It won't send any telagrams when it's not there.

![Schema](https://github.com/nldroid/CustomP1UartComponent/raw/master/p1_meter_schema.png)

## Software

Just add the .h file in your config folder and see the .yaml file for usage
