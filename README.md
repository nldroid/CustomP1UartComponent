# CustomP1UartComponent

This is my first custom component for EspHome. It can be used to read DSMR data from the P1 port of dutch smart meters with an ESP module and pubish the result in [Home Assistant](https://www.home-assistant.io/).

The work is based on these projects:
- https://esphome.io/custom/uart.html (The project where the custom component is for)
- https://github.com/rspaargaren/DSMR_ESPHOME (We shared thoughts on how to read the P1 port)
- https://github.com/matthijskooijman/arduino-dsmr (The library that i use to parse the telegrams)
- https://github.com/brandond/esphome-tuya_pir (Example how to read data from UART)
- http://domoticx.com/p1-poort-slimme-meter-uitlezen-hardware/ (Information about hardware requirements. Examples for inverters.)

## Hardware

I used a Wemos D1 mini to connect to the P1 port but it will probably work with most ESP boards. You need some kind of [hardware inverter](https://en.wikipedia.org/wiki/Inverter_(logic_gate)) because the UART component doesn't support inverting the signal with a software setting.
Port 5 from the P1 connector goes to pin 1 (A1) from the 7404. Output pin 2 from the 7404 goes to port D2 on the Wemos.
Port D5 from the Wemos is connected to port 2 from the p1 connector. This is used to request a message from the meter.
R1 is needed for my Iskra meter. It won't send any telegrams when it's not there.

## Schema
![Schema](https://raw.githubusercontent.com/nldroid/CustomP1UartComponent/master/docs/p1_meter_schema.png)

## Example bread board
![Bread board](https://raw.githubusercontent.com/nldroid/CustomP1UartComponent/master/docs/breadboard.jpg)

## Software
Just add the .h file in your config folder and see the .yaml file for usage

## Limitations
The software is only usable for meters with [8N1](https://en.wikipedia.org/wiki/8-N-1) serial communication. This is the case for newer dsmr protocols. Older procols use 7E1. You can change the software and shift the char one bit (c &= ~(1 << 7);).
Real old dsmr protocols don't have a CRC at the end of a telegram and the dsmr parser that i use, doesn't support these old protocols.

