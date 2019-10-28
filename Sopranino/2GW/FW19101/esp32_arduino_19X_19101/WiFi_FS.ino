
void Wifimanager_init(bool WiFiSerialDebug) {

  mountFS();

  parseJSON();

  printJSON();
  
  WiFi.psk().toCharArray(stored_psk, 20);
  
  WiFi.mode(WIFI_STA); // Force to station mode because if device was switched off while in access point mode it will start up next time in access point mode.
  unsigned long startedAt = millis();
  int connRes = WiFi.waitForConnectResult();
  float waited = (millis() - startedAt);
  if (WiFiSerialDebug == true) {
    Serial.print("Stored SSID: "); Serial.println(WiFi.SSID());
    Serial.print("Stored SSID (json): "); Serial.println(Tstick.lastConnectedNetwork);
    Serial.print("Stored password: "); Serial.println(WiFi.psk());
    Serial.print("After waiting "); Serial.print(waited / 1000); Serial.print(" secs in setup() connection result is "); Serial.println(connRes);
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("CONNECTION STATUS: Failed to connect up to this point, finishing setup anyway");
      Serial.println("                   If using ESP32, the T-Stick will try to connect again later");
      } 
    else {
      Serial.println("");
      Serial.print("local ip: "); Serial.println(WiFi.localIP());
      Serial.print("OSC ip: "); Serial.println(oscEndpointIP);
      Serial.print("OSC port: "); Serial.println(oscEndpointPORT);
      }
  }

  #if defined(ESP32)
    if (WiFi.status() != WL_CONNECTED) {
      if (WiFiSerialDebug == true) {Serial.println("ESP32 2nd attempt to connect");}
      WiFi.begin(Tstick.lastConnectedNetwork, stored_psk);
      if (WiFi.status() != WL_CONNECTED) {Serial.println("Connected on 2nd attempt");Serial.println(WiFi.SSID());saveLastConnectedNetwork();}
        else {Serial.println("Failed to connect, continuing setup...");}
    }
  #endif

  
  oscEndpoint.begin(portLocal);
  if (WiFiSerialDebug == true) {
    Serial.println("Starting UDP");
    Serial.print("Local port: ");
    #ifdef ESP8266
        Serial.println(oscEndpoint.localPort());
    #else
        Serial.println(portLocal);
    #endif
  }

  #if defined(ESP8266)
    WiFi.hostname(Tstick.device);
  #else
    WiFi.setHostname(Tstick.device);
  #endif
  
}

void Wifimanager_portal(char *portal_name, char *portal_password, bool WiFiSerialDebug) {

  //WiFiManager
  WiFiManager wifiManager;

  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  
//  char directSendOSCcheck[24] = "type=\"checkbox\"";
//  char calibrateCheck[24] = "type=\"checkbox\"";

  WiFiManagerParameter wifimanager_device("device", "T-Stick name (also AP SSID)", Tstick.device, 25);
  WiFiManagerParameter wifimanager_hint("<small>*Hint: if you want to reuse the currently active WiFi credentials, leave SSID and Password fields empty</small>");
  WiFiManagerParameter wifimanager_APpasswd("APpasswd", "Access Point SSID password", Tstick.APpasswd, 15);
  WiFiManagerParameter wifimanager_APpasswdValidate("APpasswdValidate", "Type password again", Tstick.APpasswd, 15);
  WiFiManagerParameter wifimanager_warning("<small>Be careful: if you forget your new password you'll not be able to connect!</small>");
//  WiFiManagerParameter custom_oscIP("server", "IP to send OSC messages", oscIP, 17);
//  WiFiManagerParameter custom_oscPORT("port", "port to send OSC messages", oscPORT, 7);
//  if (directSendOSC == 1) { strcat(directSendOSCcheck, " checked"); }
//    WiFiManagerParameter custom_directSendOSC("directSendOSC", "DirectSend OSC mode (WiFiManager exit required)", "T", 2, directSendOSCcheck, WFM_LABEL_AFTER);
//  WiFiManagerParameter custom_infoTstick0("info0", "T-Stick serial number", infoTstickCHAR0, 6);
//  WiFiManagerParameter custom_infoTstick1("info1", "T-Stick firmware revision", infoTstickCHAR1, 6);
//  if (calibrate == 1) { strcat(calibrateCheck, " checked"); }
//    WiFiManagerParameter custom_calibratetrig("calt", "Pressure sensor calibration ON/OFF (WiFiManager exit required)", "T", 2, calibrateCheck, WFM_LABEL_AFTER);
//  WiFiManagerParameter custom_calibrationData0("cal0", "Pressure sensor min calibration value (default = 0)", calibrationDataCHAR0, 6);
//  WiFiManagerParameter custom_calibrationData1("cal1", "Pressure sensor max calibration value (default = 4095)", calibrationDataCHAR1, 6);
//  WiFiManagerParameter custom_touchMaskData0("touchMask0", "Touch Mask capacitive sensing value (1/2)", touchMaskCHAR0, 7);
//  WiFiManagerParameter custom_touchMaskData1("touchMask1", "Touch Mask capacitive sensing value (2/2)", touchMaskCHAR1, 7);

  //add all your parameters here
  wifiManager.addParameter(&wifimanager_device);
  wifiManager.addParameter(&wifimanager_hint);
  wifiManager.addParameter(&wifimanager_APpasswd);
  wifiManager.addParameter(&wifimanager_APpasswdValidate);
  wifiManager.addParameter(&wifimanager_warning);
//  wifiManager.addParameter(&custom_oscIP);
//  wifiManager.addParameter(&custom_oscPORT);
//  wifiManager.addParameter(&custom_directSendOSC);
//  wifiManager.addParameter(&custom_infoTstick0);
//  wifiManager.addParameter(&custom_infoTstick1);
//  wifiManager.addParameter(&custom_calibratetrig);
//  wifiManager.addParameter(&custom_calibrationData0);
//  wifiManager.addParameter(&custom_calibrationData1);
//  wifiManager.addParameter(&custom_touchMaskData0);
//  wifiManager.addParameter(&custom_touchMaskData1);

  if (!wifiManager.startConfigPortal(portal_name, portal_password)) {
    if (WiFiSerialDebug == true) {
      Serial.println("Failed to connect and hit timeout");
    }
    delay(3000);
    Serial.println("ESP.restart");
    ESP.restart(); //reset and try again, or maybe put it to deep sleep
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  if (WiFiSerialDebug == true) {
    Serial.println("Connected to the network");
  }

  //read updated OSC and port parameters
  strcpy(Tstick.device, wifimanager_device.getValue());

  if (strcmp(wifimanager_APpasswd.getValue(), wifimanager_APpasswdValidate.getValue()) == 0 ) {
    strcpy(Tstick.APpasswd, wifimanager_APpasswd.getValue());
    }
    else {
      Serial.println("Passwords don't match!");
    }

//  strcpy(oscIP, custom_oscIP.getValue());
//  strcpy(oscPORT, custom_oscPORT.getValue());

//  strcpy(APpasswdValidate, custom_APpasswdValidate.getValue());
//  if (strncmp(custom_directSendOSC.getValue(), "T", 1) == 0) {strcpy(directSendOSCCHAR, one);}
//  else {strcpy(directSendOSCCHAR, zero);}
//  strcpy(infoTstickCHAR0, custom_infoTstick0.getValue());
//  strcpy(infoTstickCHAR1, custom_infoTstick1.getValue());
//  if (strncmp(custom_calibratetrig.getValue(), "T", 1) == 0) {strcpy(calibrateCHAR, one);}
//  else {strcpy(calibrateCHAR, zero);}
//  strcpy(calibrationDataCHAR0, custom_calibrationData0.getValue());
//  strcpy(calibrationDataCHAR1, custom_calibrationData1.getValue());
//  strcpy(touchMaskCHAR0, custom_touchMaskData0.getValue());
//  strcpy(touchMaskCHAR1, custom_touchMaskData1.getValue());

  // validating some fields
//  if (strcmp(APpasswdTemp, APpasswdValidate) == 0 ) {
//    strcpy(APpasswd, APpasswdTemp);
//  }
//  else {
//    strcpy(APpasswdTemp, APpasswd);
//    strcpy(APpasswdValidate, APpasswd);
//  }

  saveJSON(DEBUG);  //save the custom parameters to FS
 
  if (WiFiSerialDebug == true) {
    Serial.println("");
    Serial.print("local ip: "); Serial.println(WiFi.localIP());
    Serial.print("OSC ip: "); Serial.println(oscEndpointIP);
    Serial.print("OSC port: "); Serial.println(oscEndpointPORT);
  }
  digitalWrite(ledPin, LOW);
}


void mountFS() {

  SPIFFS.begin (true);
  
  if (SPIFFS.begin()) {
    Serial.println("\nFile system mounted!");
    } 
  else {
    Serial.println("Failed to mount file system.\n");
    }
}
    
    
void printJSON() {
  if (SPIFFS.exists("/config.json")) { // file exists, reading and loading
    Serial.println("Reading config file...");
    File configFile = SPIFFS.open("/config.json", "r");   // Open file for reading
      if (configFile) {
        Serial.println("Config file opened:\n");
        while(configFile.available()){Serial.write(configFile.read());}
        Serial.println("\n\n");
        }
      else {
        Serial.println("Cannot read config file config.json. File doesn't exist.\n");
        }
  }
}

void parseJSON() {    
  // Allocate a temporary JsonDocument
  const size_t capacity = 2*JSON_ARRAY_SIZE(2) + 3*JSON_ARRAY_SIZE(3) + 3*JSON_ARRAY_SIZE(9) + JSON_OBJECT_SIZE(18) + 270;
  DynamicJsonDocument doc(capacity);

  if (SPIFFS.exists("/config.json")) { // file exists, reading and loading
    Serial.println("Reading config file...");
    File configFile = SPIFFS.open("/config.json");
    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, configFile);
      if (error) {
        Serial.println("Failed to read file!\n");
        } 
      else {
        // Copy values from the JsonDocument to the Config
        strlcpy(Tstick.device, doc["device"], sizeof(Tstick.device));
        strlcpy(Tstick.author, doc["author"], sizeof(Tstick.author));
        strlcpy(Tstick.nickname, doc["nickname"], sizeof(Tstick.nickname));
        strlcpy(Tstick.APpasswd, doc["APpasswd"], sizeof(Tstick.APpasswd));
        strlcpy(Tstick.lastConnectedNetwork, doc["lastConnectedNetwork"], sizeof(Tstick.lastConnectedNetwork));
        Tstick.id = doc["id"];
        Tstick.firmware = doc["firmware"];
        strlcpy(Tstick.oscIP, doc["oscIP"], sizeof(Tstick.oscIP));
        Tstick.oscPORT = doc["oscPORT"];
        Tstick.directSend = doc["directSend"];
        Tstick.FSRcalibration = doc["FSRcalibration"];
        for( int i = 0 ; i < (sizeof(Tstick.FSRcalibrationValues)/sizeof(Tstick.FSRcalibrationValues[0])) ; ++i ){
          Tstick.FSRcalibrationValues[i] = doc["FSRcalibrationValues"][i];
        }
        for( int i = 0 ; i < (sizeof(Tstick.touchMask)/sizeof(Tstick.touchMask[0])) ; ++i ){
          Tstick.touchMask[i] = doc["touchMask"][i];
        }
        for( int i = 0 ; i < (sizeof(Tstick.abias)/sizeof(Tstick.abias[0])) ; ++i ){
          Tstick.abias[i] = doc["abias"][i];
        }
        for( int i = 0 ; i < (sizeof(Tstick.mbias)/sizeof(Tstick.mbias[0])) ; ++i ){
          Tstick.mbias[i] = doc["mbias"][i];
        }
        for( int i = 0 ; i < (sizeof(Tstick.gbias)/sizeof(Tstick.gbias[0])) ; ++i ){
          Tstick.gbias[i] = doc["gbias"][i];
        }
        for( int i = 0 ; i < (sizeof(Tstick.acclcalibration)/sizeof(Tstick.acclcalibration[0])) ; ++i ){
          Tstick.acclcalibration[i] = doc["acclcalibration"][i];
        }
        for( int i = 0 ; i < (sizeof(Tstick.magncalibration)/sizeof(Tstick.magncalibration[0])) ; ++i ){
          Tstick.magncalibration[i] = doc["magncalibration"][i];
        }
        for( int i = 0 ; i < (sizeof(Tstick.gyrocalibration)/sizeof(Tstick.gyrocalibration[0])) ; ++i ){
          Tstick.gyrocalibration[i] = doc["gyrocalibration"][i];
        }
        configFile.close();
        Serial.println("T-Stick configuration file loaded.\n");
        }
      
    } 
  else {
    Serial.println("Failed to read config file!\n");
    }
}


void saveJSON(bool WiFiSerialDebug) {
  
  if (WiFiSerialDebug == true) {
    Serial.println("Saving config to JSON file...");
  }
  // Allocate a temporary JsonDocument
  const size_t capacity = 2*JSON_ARRAY_SIZE(2) + 3*JSON_ARRAY_SIZE(3) + 3*JSON_ARRAY_SIZE(9) + JSON_OBJECT_SIZE(18) + 270;
  DynamicJsonDocument doc(capacity);

  // Copy values from Config to the JsonDocument
  doc["device"] = Tstick.device;
  doc["author"] = Tstick.author;
  doc["nickname"] = Tstick.nickname;
  doc["APpasswd"] = Tstick.APpasswd;
  doc["lastConnectedNetwork"] = Tstick.lastConnectedNetwork;
  doc["id"] = Tstick.id;
  doc["firmware"] = Tstick.firmware;
  doc["oscIP"] = Tstick.oscIP;
  doc["oscPORT"] = Tstick.oscPORT;
  doc["directSend"] = Tstick.directSend;
  doc["FSRcalibration"] = Tstick.FSRcalibration;
  for( int i = 0 ; i < (sizeof(Tstick.FSRcalibrationValues)/sizeof(Tstick.FSRcalibrationValues[0])) ; ++i ){
    doc["FSRcalibrationValues"][i] = Tstick.FSRcalibrationValues[i];
  }
  for( int i = 0 ; i < (sizeof(Tstick.touchMask)/sizeof(Tstick.touchMask[0])) ; ++i ){
    doc["touchMask"][i] = Tstick.touchMask[i];
  }
  for( int i = 0 ; i < (sizeof(Tstick.abias)/sizeof(Tstick.abias[0])) ; ++i ){
    doc["abias"][i] = Tstick.abias[i];
  }
  for( int i = 0 ; i < (sizeof(Tstick.mbias)/sizeof(Tstick.mbias[0])) ; ++i ){
    doc["mbias"][i] = Tstick.mbias[i];
  }
  for( int i = 0 ; i < (sizeof(Tstick.gbias)/sizeof(Tstick.gbias[0])) ; ++i ){
    doc["gbias"][i] = Tstick.gbias[i];
  }
  for( int i = 0 ; i < (sizeof(Tstick.acclcalibration)/sizeof(Tstick.acclcalibration[0])) ; ++i ){
    doc["acclcalibration"][i] = Tstick.acclcalibration[i];
  }
  for( int i = 0 ; i < (sizeof(Tstick.magncalibration)/sizeof(Tstick.magncalibration[0])) ; ++i ){
    doc["magncalibration"][i] = Tstick.magncalibration[i];
  }
  for( int i = 0 ; i < (sizeof(Tstick.gyrocalibration)/sizeof(Tstick.gyrocalibration[0])) ; ++i ){
    doc["gyrocalibration"][i] = Tstick.gyrocalibration[i];
  }

  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    if (WiFiSerialDebug == true) {
      Serial.println("Failed to open config file for writing!\n");
    }
  }
  // Serialize JSON to file
  if (serializeJson(doc, configFile) == 0) {Serial.println("Failed to write to file");}
  else {Serial.println("JSON file successfully saved!");}
  configFile.close();
  //end save
}


void saveLastConnectedNetwork() {

  Serial.println("Saving Last Connected Network to JSON file...");

  // Allocate a temporary JsonDocument
  const size_t capacity = 2*JSON_ARRAY_SIZE(2) + 3*JSON_ARRAY_SIZE(3) + 3*JSON_ARRAY_SIZE(9) + JSON_OBJECT_SIZE(18) + 270;
  DynamicJsonDocument doc(capacity);

  // Copy values from Config to the JsonDocument
  doc["device"] = Tstick.device;
  doc["author"] = Tstick.author;
  doc["nickname"] = Tstick.nickname;
  doc["APpasswd"] = Tstick.APpasswd;
  doc["lastConnectedNetwork"] = Tstick.lastConnectedNetwork;
  doc["id"] = Tstick.id;
  doc["firmware"] = Tstick.firmware;
  doc["oscIP"] = Tstick.oscIP;
  doc["oscPORT"] = Tstick.oscPORT;
  doc["directSend"] = Tstick.directSend;
  doc["FSRcalibration"] = Tstick.FSRcalibration;
  for( int i = 0 ; i < (sizeof(Tstick.FSRcalibrationValues)/sizeof(Tstick.FSRcalibrationValues[0])) ; ++i ){
    doc["FSRcalibrationValues"][i] = Tstick.FSRcalibrationValues[i];
  }
  for( int i = 0 ; i < (sizeof(Tstick.touchMask)/sizeof(Tstick.touchMask[0])) ; ++i ){
    doc["touchMask"][i] = Tstick.touchMask[i];
  }
  for( int i = 0 ; i < (sizeof(Tstick.abias)/sizeof(Tstick.abias[0])) ; ++i ){
    doc["abias"][i] = Tstick.abias[i];
  }
  for( int i = 0 ; i < (sizeof(Tstick.mbias)/sizeof(Tstick.mbias[0])) ; ++i ){
    doc["mbias"][i] = Tstick.mbias[i];
  }
  for( int i = 0 ; i < (sizeof(Tstick.gbias)/sizeof(Tstick.gbias[0])) ; ++i ){
    doc["gbias"][i] = Tstick.gbias[i];
  }
  for( int i = 0 ; i < (sizeof(Tstick.acclcalibration)/sizeof(Tstick.acclcalibration[0])) ; ++i ){
    doc["acclcalibration"][i] = Tstick.acclcalibration[i];
  }
  for( int i = 0 ; i < (sizeof(Tstick.magncalibration)/sizeof(Tstick.magncalibration[0])) ; ++i ){
    doc["magncalibration"][i] = Tstick.magncalibration[i];
  }
  for( int i = 0 ; i < (sizeof(Tstick.gyrocalibration)/sizeof(Tstick.gyrocalibration[0])) ; ++i ){
    doc["gyrocalibration"][i] = Tstick.gyrocalibration[i];
  }

  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing!\n");
  }
  // Serialize JSON to file
  if (serializeJson(doc, configFile) == 0) {Serial.println("Failed to write to file");}
  else {Serial.println("JSON file successfully saved!");}
  configFile.close();
  //end save
}


void printVariables() {
  Serial.println("Printing loaded values:\n");
  Serial.print("Tstick.device: "); Serial.println(Tstick.device);
  Serial.print("Tstick.author: "); Serial.println(Tstick.author);
  Serial.print("Tstick.nickname: "); Serial.println(Tstick.nickname);
  Serial.print("Tstick.APpasswd: "); Serial.println(Tstick.APpasswd);
  Serial.print("Tstick.lastConnectedNetwork: "); Serial.println(Tstick.lastConnectedNetwork);
  Serial.print("Tstick.id: "); Serial.println(Tstick.id);
  Serial.print("Tstick.firmware: "); Serial.println(Tstick.firmware);
  Serial.print("Tstick.oscIP: "); Serial.println(Tstick.oscIP);
  Serial.print("Tstick.oscPORT: "); Serial.println(Tstick.oscPORT);
  Serial.print("Tstick.directSend: "); Serial.println(Tstick.directSend);
  Serial.print("Tstick.FSRcalibration: "); Serial.println(Tstick.FSRcalibration);
  Serial.print("Tstick.FSRcalibrationValues: ");
  for( int i = 0 ; i < (sizeof(Tstick.FSRcalibrationValues)/sizeof(Tstick.FSRcalibrationValues[0])) ; ++i ){
    Serial.print(Tstick.FSRcalibrationValues[i]);
    Serial.print(" ");
  }
  Serial.println(" ");
  Serial.print("Tstick.touchMask: ");
  for( int i = 0 ; i < (sizeof(Tstick.touchMask)/sizeof(Tstick.touchMask[0])) ; ++i ){
    Serial.print(Tstick.touchMask[i]);
    Serial.print(" ");
  }
  Serial.println(" ");
  Serial.print("Tstick.abias: ");
  for( int i = 0 ; i < (sizeof(Tstick.abias)/sizeof(Tstick.abias[0])) ; ++i ){
    Serial.print(Tstick.abias[i], 10);
    Serial.print(" ");
  }
  Serial.println(" ");
  Serial.print("Tstick.mbias: ");
  for( int i = 0 ; i < (sizeof(Tstick.mbias)/sizeof(Tstick.mbias[0])) ; ++i ){
    Serial.print(Tstick.mbias[i], 10);
    Serial.print(" ");
  }
  Serial.println(" ");
  Serial.print("Tstick.gbias: ");
  for( int i = 0 ; i < (sizeof(Tstick.gbias)/sizeof(Tstick.gbias[0])) ; ++i ){
    Serial.print(Tstick.gbias[i], 10);
    Serial.print(" ");
  }
  Serial.println(" ");
  Serial.print("Tstick.acclcalibration: ");
  for( int i = 0 ; i < (sizeof(Tstick.acclcalibration)/sizeof(Tstick.acclcalibration[0])) ; ++i ){
    Serial.print(Tstick.acclcalibration[i], 10);
    Serial.print(" ");
  }
  Serial.println(" ");
  Serial.print("Tstick.magncalibration: ");
  for( int i = 0 ; i < (sizeof(Tstick.magncalibration)/sizeof(Tstick.magncalibration[0])) ; ++i ){
    Serial.print(Tstick.magncalibration[i], 10);
    Serial.print(" ");
  }
  Serial.println(" ");
  Serial.print("Tstick.gyrocalibration: ");
  for( int i = 0 ; i < (sizeof(Tstick.gyrocalibration)/sizeof(Tstick.gyrocalibration[0])) ; ++i ){
    Serial.print(Tstick.gyrocalibration[i], 10);
    Serial.print(" ");
  }
  Serial.println();
}
