# T-Stick 2GW (Firmware version: 19101)

//********************************************************************************//
//  Sopranino T-Stick 2GW - LOLIN D32 PRO - USB -WiFi                             //
//  Input Devices and Music Interaction Laboratory (IDMIL)                        //
//  Created:  February 2018 by Alex Nieva                                         //
//            October 2019 by Edu Meneses - firmware version 19101 (2019-10.v1)   //
//  Notes   : Based on test program for reading CY8C201xx using I2C               //
//            by Joseph Malloch 2011                                              //
//                                                                                //
//            Adapted to work with Arduino IDE 1.8.10 and T-Stick Sopranino 2GW   //
//********************************************************************************//

//**************************************************//
// WiFi32Manager - For use with ESP8266 or ESP32    //
//                                                  //
// Created originally for the T-Stick project:      //
// http://www-new.idmil.org/project/the-t-stick/    //
//                                                  //
// This code uses code (fork) of 3 other projects:  //
// https://github.com/kentaylor/WiFiManager         //
// https://github.com/tzapu/WiFiManager             //
// https://github.com/zhouhan0126/WIFIMANAGER-ESP32 //
//                                                  //
// Edu Meneses - IDMIL - Mar 2019                   //
//**************************************************//

//**************************************************//
// MIMU - Magnetometer + accelerometer + gyroscope  //
// orientation library                              //
//                                                  //
// https://github.com/DocSunset/MIMU                //
//                                                  //
//                                                  //
// Travis West - IDMIL - Oct 2019                   //
//**************************************************//

## Firmware upload instructions:

#### Option 1: using .bin files and esptool.py

TODO

#### Option 2: Using Arduino IDE

_READ ALL DEPENDENCIES AND OBSERVATIONS BEFORE UPLOAD !_

###### Install Arduino IDE:

To download and install Arduino IDE, follow the instructions at https://www.arduino.cc/en/main/software.

###### Install Arduino ESP32 filesystem uploader: 

You need to upload a file (data/config.json) into ESP32 filesystem. 
Follow the instructions at https://github.com/me-no-dev/arduino-esp32fs-plugin

###### Install all depencencies:

1. ESP32 Arduino core 1.0.4 or newer. Instructons at https://github.com/espressif/arduino-esp32/blob/master/docs/arduino-ide/boards_manager.md. Install using [boards manager](https://www.arduino.cc/en/guide/cores).
2. For some systems it may be required to also install the ESP8266 Arduino core. Instructons at https://github.com/esp8266/Arduino. Install using [boards manager](https://www.arduino.cc/en/guide/cores).
3. [Wifi32Manager](https://github.com/edumeneses/WiFi32Manager). Instructons at https://github.com/edumeneses/WiFi32Manager#installing. Install using [manual installation](https://www.arduino.cc/en/guide/libraries#toc5).
4. [Adafruit_LSM9DS1](https://github.com/adafruit/Adafruit_LSM9DS1). Install using [library manager](https://www.arduino.cc/en/guide/libraries#toc3).
5. [ArduinoJSON](https://github.com/bblanchon/ArduinoJson) v6.12 or up. Install using [library manager](https://www.arduino.cc/en/guide/libraries#toc3).
6. [CNMAT OSC library](https://github.com/CNMAT/OSC) v3.5.7 or up. Install using [library manager](https://www.arduino.cc/en/guide/libraries#toc3).
7. [DocSunset MIMU Library](https://github.com/DocSunset/MIMU). Install using [manual installation](https://www.arduino.cc/en/guide/libraries#toc5).
8. [Eigen linear algebra library for Arduino](https://github.com/bolderflight/Eigen). Install using [manual installation](https://www.arduino.cc/en/guide/libraries#toc5).

Observations:

1. MINU library complains if you keep any IMU-related files other than MIMU_LSM9DS1.h and MIMU_LSM9DS1.cpp
2. Microcontroller currently in use for T-Stick 2GW: [Wemos LOLIN D32 PRO](https://wiki.wemos.cc/products:d32:d32_pro).

###### Create your custom config.json file:

Each T-Stick uses a _config.json_ file to store all configuration paramethers. The template file can be found [here](./datafiles/config(template).json).

- Make a copy of [_config(template).json_](./datafiles/config(template).json) and rename it to _config.json_;
- Make the necessary changes to the file:
  - device: replace _color_ with the shrinking material color, and _19X_ with T-stick's serial number
  - author: replace _IDMIL_ with the builder's name (or alias) 
  - nickname: replace _color_ with the shrinking material color
  - id: replace _190_ with your T-Stick serial number
- Save the file and place it at _data_ folder (inside _esp32_arduino_19X_19101_). Make sure _config.json_ is the only file at _data_ folder

###### Upload (flash) the firmware and config.json into the T-Stick:

- Open the file `esp32_arduino_19X_19101.ino` using Arduino IDE
- Choose the proper _board_: `Tools -> Board: "*****" -> LOLIN D32 PRO`
- Choose Upload Speed: `Tools -> Upload Speed: "****" -> 115200`
- Choose port:
  - Keep the T-Stick disconnected to the computer
  - Go to: `Tools -> Port` and take note the available ports
  - connect the T-Stick to the computer using an USB cable and turn the T-Stick ON
  - Go to: `Tools -> Port` and choose the new available port (T-Stick port)
- Upload the firmware: `Sketch -> Upload`. Do not disconnect or turn the T-Stick off during the upload process
- Upload _config.json_: `Tools -> ESP32 Sketch Data Upload`. Do not disconnect or turn the T-Stick off during the upload process

###### Test T-Stick:

To test your T-Stick after flashing firmware/config.json, you can use Arduino IDE:

- Choose T-Stick serial port
- Open the _Serial Monitor_ (`Tools -> Serial Monitor`)

You should see T-Stick booting process.

To test if the data is being send correctly:

- Connect the T-Stick to a network (instructions [here](./Sopranino/2GW/FW19101/Docs/T-Stick_2GW_Connecting_Guide(v1.1).md))
- Open the Pure Data (PD) or Max/MSP patch to receive T-Stick messages (they can be found [here](.Configuration))
- Start receive OSC messages according to the chosen patch

## Other Documentation:

[T-Stick connection guide – v1.1 for wireless T-Sticks](./Sopranino/2GW/FW19101/Docs/T-Stick_2GW_Connecting_Guide(v1.1).md) (model 2GW-19X)

[How to build a T-Stick Sopranino](./Sopranino/2GW/FW19101/Docs/T-Stick_2GW_building_instructions.md)