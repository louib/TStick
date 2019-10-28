

byte OSCMsgReceive()
{
  //Serial.println("Receiving OSC...");
  OSCErrorCode error;
  
  boolean changed = 0;
  OSCMessage msgRec;

  int size = oscEndpoint.parsePacket();
  //Serial.println(size);

  if (size > 0) {
    if (DEBUG) {
      Serial.printf("Received %d bytes from %s, port %d\n", size, oscEndpoint.remoteIP().toString().c_str(), oscEndpoint.remotePort());
    }
    while (size--) {
      msgRec.fill(oscEndpoint.read());
    }
    if (!msgRec.hasError()) {
      error = msgRec.getError();
      Serial.print("error: ");
      Serial.println(error);
      changed = 1;
      msgRec.route("/status", fromHost);
      msgRec.route("/state/calibrate", saveIMUcalib);
    }
  }
  return changed;
}

void fromHost(OSCMessage &msg, int addressOffset) {
  int S = msg.size();
  if (DEBUG) {
    int L = msg.getDataLength(0);
    char type = msg.getType(0);
    Serial.print("Size total msg: "); Serial.println(S);
    Serial.print("Size of msg: "); Serial.println(L);
    Serial.print("Type of msg: "); Serial.println(type);
  }
  for (int i = 0; i < S; i++)
  {
    bufferFromHost[i] = msg.getInt(i);
  }
}

void saveIMUcalib(OSCMessage &msg, int addressOffset) {
  // message order: avector[3], amatrix[9], mvector[3], mmatrix[9], gvector[3], gmatrix[9]
  for (byte i = 0; i < 3; i++) {
    Tstick.abias[i] = msg.getFloat(i);
    Tstick.mbias[i+12] = msg.getFloat(i+12);
    Tstick.gbias[i+24] = msg.getFloat(i+24);
  }
  for (byte i = 0; i < 9; i++) {
    Tstick.acclcalibration[i+3] = msg.getFloat(i+3);
    Tstick.magncalibration[i+15] = msg.getFloat(i+15);
    Tstick.gyrocalibration[i+27] = msg.getFloat(i+27);
  }

  saveJSON(true);  
}

void TStickReceiveRoutine() {
    
  byte dataRec = OSCMsgReceive();

  if (dataRec) { //Check for OSC messages from host computers
    if (DEBUG) {
      Serial.println();
      for (int i = 0; i < 4; i++) {
        Serial.printf("From computer %d: ", i); Serial.println(bufferFromHost[i]);
      }
      Serial.println();
    }
    char message = bufferFromHost[0];

    OSCMessage msg0("/information");

    switch (message) {
      case 's': // start message,
        msg0.add(Tstick.id);
        msg0.add(Tstick.firmware);
        oscEndpoint.beginPacket(oscEndpointIP, oscEndpointPORT);
        msg0.send(oscEndpoint);
        oscEndpoint.endPacket();
        msg0.empty();

        started = millis();
        break;
      case 'x': // stop message,
        started = 0;
        break;
      case 'c': // calibrate message
        switch (bufferFromHost[1]) {
          case 1: // FSR calibration
            Tstick.FSRcalibrationValues[0] = bufferFromHost[2];
            Tstick.FSRcalibrationValues[1] = bufferFromHost[3];
            Tstick.FSRcalibration = 1;
            bufferFromHost[1] = 0;
            bufferFromHost[2] = 0;
            bufferFromHost[3] = 0;
            break;
          default:
            Tstick.FSRcalibration = 0;
            break;
        }
        break;
      case 'w':     //write settings
        switch ((char)bufferFromHost[1]) {
          case 'i': //write info
            Tstick.id = bufferFromHost[2];
            Tstick.firmware = bufferFromHost[3];
            bufferFromHost[1] = 0;
            bufferFromHost[2] = 0;
            bufferFromHost[3] = 0;
            break;
          case 'T': //write touch mask
            Tstick.touchMask[0] = bufferFromHost[2];
            Tstick.touchMask[1] = bufferFromHost[3];
            bufferFromHost[1] = 0;
            bufferFromHost[2] = 0;
            bufferFromHost[3] = 0;
            break;
          case 'w': // write settings to memory (json)
            saveJSON(DEBUG);
            bufferFromHost[1] = 0;
            break;
          case 'r': // sending the config info trough OSC
            msg0.add(Tstick.id);
            msg0.add(Tstick.firmware);
            msg0.add(Tstick.FSRcalibration);
            msg0.add(Tstick.FSRcalibrationValues[0]);
            msg0.add(Tstick.FSRcalibrationValues[1]);
            msg0.add(Tstick.touchMask[0]);
            msg0.add(Tstick.touchMask[1]);
            oscEndpoint.beginPacket(oscEndpointIP, oscEndpointPORT);
            msg0.send(oscEndpoint);
            oscEndpoint.endPacket();
            msg0.empty();
            bufferFromHost[1] = 0;
            break;
          default:
            break;
        }
      default:
        break;
    }
  }
}
