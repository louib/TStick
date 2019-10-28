

boolean readTouch(){
  boolean changed = 0;
  byte temp[2] = {0, 0}; int i=0;
  Wire.beginTransmission(I2C_ADDR);
  Wire.write(BUTTON_STAT);
  Wire.endTransmission();

  Wire.requestFrom(I2C_ADDR,2);
  while (Wire.available()) { // slave may send less than requested
//    byte c = Wire.read();
//    temp[i] = c; // receive a byte as character
    temp[i] = Wire.read();
    i++;
  }    
  Wire.endTransmission();

  for (int t = 0; t<2; t++){
    if (temp[t] != touch[t]){
      changed = 1;
      touch[t] = temp[t];
    }
  }
  return changed;
}

void addFloatArrayToMessage(const float * const v, int size, OSCMessage& m) {
  for (int i = 0; i < size; ++i) m.add(*(v+i));
}

void TStickRoutine() {
  if (sendOSC) {
    static OSCBundle bundle;
    if (millis()-lastRead>touchInterval) {
      lastRead = millis();
      if (readTouch()){
        OSCMessage msg1("/rawcapsense");
        msg1.add((int)touch[0] & Tstick.touchMask[0]);
        msg1.add((int)touch[1] & Tstick.touchMask[1]);
        bundle.add(msg1);
      }
    }
    
    static MIMUReading reading = MIMUReading::Zero();
    static Quaternion quat = Quaternion::Identity();
    if (mimu.readInto(reading)) 
    {
      calibrator.calibrate(reading);
      reading.updateBuffer();
      quat = filter.fuse(reading.gyro, reading.accl, reading.magn);
    
      OSCMessage msg2("/raw/gyro");
      addFloatArrayToMessage(reading.gyro.data(), reading.gyro.size(), msg2);
      bundle.add(msg2);
      
      OSCMessage msg3("/raw/accel");
      addFloatArrayToMessage(reading.accl.data(), reading.accl.size(), msg3);
      bundle.add(msg3);
      
      OSCMessage msg4("/raw/mag");
      addFloatArrayToMessage(reading.magn.data(), reading.magn.size(), msg4);
      bundle.add(msg4);
  
      OSCMessage msg5("/raw");
      addFloatArrayToMessage(reading.data, reading.size, msg5);
      bundle.add(msg5);
  
      OSCMessage msg6("/orientation");
      addFloatArrayToMessage(quat.coeffs().data(), quat.coeffs().size(), msg6);
      bundle.add(msg6);

    }
    
    int pressure = analogRead(pressurePin);
    if (Tstick.FSRcalibration == 1) {
      pressure = map(pressure, Tstick.FSRcalibrationValues[0], Tstick.FSRcalibrationValues[1], 0, 1024);
      if (pressure < 0) {pressure = 0;} 
     // pressure = constrain(pressure, 0, 4095);
    }
    OSCMessage msg7("/raw/pressure");
    msg7.add(pressure);
    bundle.add(msg7);
  
    unsigned int piezo = analogRead(piezoPin);
//    if (calibrate == 1) {
//      calibrationData[0] = constrain(min(calibrationData[0], piezo), 0, 4095);
//      calibrationData[1] = constrain(max(calibrationData[1], piezo), 0, 4095);
//    }
//    piezo = constrain(map(piezo, calibrationData[0], calibrationData[1], 0, 4095), 0, 4095);
    OSCMessage msg8("/raw/piezo");
    msg8.add(piezo);
    bundle.add(msg8);
  
    oscEndpoint.beginPacket(oscEndpointIP, oscEndpointPORT);
    bundle.send(oscEndpoint);
    oscEndpoint.endPacket();
    bundle.empty();
  }  

  ledBlink();
  then = now;
}

void ledBlink() {
  ledStatus = (ledStatus + 1) % 2;
  digitalWrite(ledPin, ledStatus);
}
