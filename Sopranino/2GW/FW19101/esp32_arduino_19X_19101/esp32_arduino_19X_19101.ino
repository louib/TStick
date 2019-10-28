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


// READ ALL DEPENDENCIES AND OBSERVATIONS BEFORE UPLOAD !!!!!!!!
// ---- --- ------------ --- ------------ ------ ------ 
// ---- --- ------------ --- ------------ ------ ------ 


// IMPORTANT: You need to upload a file (data/config.json) into ESP32 filesystem. 
// Follow the instructions at https://github.com/me-no-dev/arduino-esp32fs-plugin

// DEPENDENCIES:
// esp32 board library (add url to preferences; install via board manager)
// esp8266 board library (add url to preferences; install via board manager)
// Wifi32Manager https://github.com/edumeneses/WiFi32Manager
// Adafruit_LSM9DS1 (library manager)
// ArduinoJSON - https://github.com/bblanchon/ArduinoJson
// OSC - https://github.com/CNMAT/OSC (library manager)
// MIMU - https://github.com/DocSunset/MIMU
// Eigen - https://github.com/bolderflight/Eigen

//  OBS:
//  1-) Use esp32 1.0.4 or newer (https://github.com/espressif/arduino-esp32/releases).
//  2-) Also install ESP8266 board library even if you'll use the ESP32 (https://github.com/esp8266/Arduino)
//  3-) MUMI library complains if you keep any IMU-related files other than MIMU_LSM9DS1.h and MIMU_LSM9DS1.cpp
//  4-) Board currently in use: LOLIN D32 PRO.


#include <FS.h>  // this needs to be first, or it all crashes and burns...

#define ESP32; // define ESP8266 or ESP32 according to your microcontroller.
//#define ESP8266;

#if defined(ESP32)
#include "SPIFFS.h"
#endif

/* You only need to format SPIFFS the first time you run a
   test or else use the SPIFFS plugin to create a partition
   https://github.com/me-no-dev/arduino-esp32fs-plugin */
//#define FORMAT_SPIFFS_IF_FAILED true

#include <WiFi32Manager.h> // https://github.com/edumeneses/WiFi32Manager
// already includes:
// Wifi.h (https://github.com/esp8266/Arduino) or ESP8266WiFi.h (https://github.com/esp8266/Arduino)
// AND
// WebServer.h or ESP8266WebServer.h
// AND
// DNSServer.h

#include <ArduinoJson.h>  // https://github.com/bblanchon/ArduinoJson

#include <Wire.h>
#include <WiFiUdp.h>

// https://github.com/CNMAT/OSC
#include <OSCMessage.h>
#include <OSCBundle.h>

#include <SPI.h>

#include <MIMU_LSM9DS1.h> // https://github.com/DocSunset/MIMU
                          // requires SparkFunLSM9DS1 library - https://github.com/sparkfun/SparkFun_LSM9DS1_Arduino_Library
#include <MIMUCalibrator.h>
#include <MIMUFusion.h>


//////////////////////////////////
//////////////////////////////////
// T-Stick Specific Definitions //
//////////////////////////////////
//////////////////////////////////

struct Tstick {
  char device[25];
  char author[20];
  char nickname[10];
  char APpasswd[15];
  char lastConnectedNetwork[30];
  int id;
  int32_t firmware;
  char oscIP[17];
  int32_t oscPORT;
  byte directSend;
  byte FSRcalibration;
  int FSRcalibrationValues[2];
  byte touchMask[2];
  double abias[3];
  double mbias[3];
  double gbias[3];
  double acclcalibration[9];
  double magncalibration[9];
  double gyrocalibration[9];
};

Tstick Tstick;

// Debug & calibration definitions
#define DEBUG true
#define CALIB false


//////////////////////
// WiFi Definitions //
//////////////////////

//define your default values here, if there are different values in config.json, they are overwritten.
IPAddress oscEndpointIP(192, 168, 1, 1); // remote IP of your computer
unsigned int oscEndpointPORT = 8000; // remote port to receive OSC
int timeout1 = 5000; bool timeout1check = false;
WiFiUDP oscEndpoint;            // A UDP instance to let us send and receive packets over UDP
const unsigned int portLocal = 8888;       // local port to listen for OSC packets (actually not used for sending)
bool udpConnected = false;
bool sendOSC = true;
static int bufferFromHost[4] = {0, 0, 0, 0};
int interTouch[2];
char zero[3] = "0";
char one[3] = "1";
char stored_psk[20];


///////////////////////
// MIMU Library Init //
///////////////////////

MIMU_LSM9DS1 mimu{}; // use default SDA and SCL as per board library macros
MIMUCalibrator calibrator{};
MIMUFusionFilter filter{};


////////////////////////////
// Sketch Output Settings //
////////////////////////////

#define PRINT_CALCULATED
//#define PRINT_RAW
#define PRINT_SPEED 20 // 250 ms between prints
static unsigned long lastPrint = 0; // Keep track of print time


//////////////
// defaults //
//////////////

int piezoPin = 32;
int pressurePin = 33;
int ledPin = 5; //changed for The thing dev during testing
int ledStatus = 0;
int ledTimer = 1000;
byte touch[2] = {0, 0};
unsigned int pressure = 0;
const int buttonPin = 15;


////////////////////////
//control definitions //
////////////////////////

unsigned long then = 0;
unsigned long now = 0;
unsigned long started = 0;
unsigned long lastRead = 0;
byte interval = 10;
byte touchInterval = 15;
unsigned long lastReadSensors = 0;
int buttonState = 0; // variable for reading the pushbutton status


void setup() {

  
  //wifiManager.setDebugOutput(true);

  Serial.begin(115200);
  if (DEBUG == true) {
    Serial.println("\n Starting");
  }

  // Converting T-Stick info into char (str)
//  itoa(Tstick.id, infoTstickCHAR0, 10);
//  itoa(Tstick.firmware, infoTstickCHAR1, 10);
//  strcat(device, nickname);
//  strcat(device, "_");
//  strcat(device, infoTstickCHAR0);
//  memcpy(APpasswdValidate, APpasswd, 15);
//  memcpy(APpasswdTemp, APpasswd, 15);

  Serial.println("\n");
  Serial.println("*******************************************************************************");
  Serial.println("*  Sopranino T-Stick 2GW - LOLIN D32 PRO - USB -WiFi                          *");
  Serial.println("*  Input Devices and Music Interaction Laboratory (IDMIL)                     *");
  Serial.println("*  Created: February 2018 by Alex Nieva                                       *");
  Serial.println("*           October 2019 by Edu Meneses - firmware version 19101 (2019-10.v1) *");
  Serial.println("*  Notes:   Based on test program for reading CY8C201xx using I2C             *");
  Serial.println("*           by Joseph Malloch 2011                                            *");
  Serial.println("                                                                              *");
  Serial.println("*  Adapted to work with Arduino IDE 1.8.10 and T-Stick Sopranino 2GW          *");
  Serial.println("*******************************************************************************");
  Serial.println("\n");

  
  // Starting WiFiManager in Trigger Mode
  Wifimanager_init(DEBUG);

  // Starting IMU
  initIMU();

  // Starting Capsense
  initCapsense();

  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  delay(100);

  if (DEBUG == true) {
    Serial.println("Setup complete.");
  }

  #if defined(ESP32) 
    if (WiFi.status() != WL_CONNECTED) {
      if (DEBUG == true) {Serial.println("ESP32 3rd attempt to connect");}
      WiFi.begin(Tstick.lastConnectedNetwork, stored_psk);
      if (DEBUG == true) {
        if (WiFi.status() != WL_CONNECTED) {Serial.println("Connected on 3rd attempt");saveLastConnectedNetwork();}
        else {Serial.println("Failed to connect, finishing setup anyway");}
      }
    }
  #endif
  
}

void loop() {

  
  // Calling WiFiManager configuration portal on demand:
  buttonState = digitalRead(buttonPin);
  if ( buttonState == LOW ) {
    digitalWrite(ledPin, HIGH);
    Wifimanager_portal(Tstick.device, Tstick.APpasswd, DEBUG);
  }

  // Receiving OSC messages:
  TStickReceiveRoutine();

  now = millis();

  // Sending OSC messages:
  if (Tstick.directSend == 0) { // needs a ping/keep-alive every 2 seconds or less or will time-out
    if (now < started + 2000) {TStickRoutine();}
    else { 
      now = millis();
      if ((now - then) > ledTimer) {ledBlink(); then = now;}
      else if ( then > now) {then = 0;}
      }
    } 
  else { TStickRoutine();} // Direct OSC mode

} // END LOOP
