bool FilesInitializeFiles() {
  if(!SPIFFS.begin(true)){
    return false;
  }
  return true;
}

bool FilesWriteOperationTimes(WiFiInsertion wakeTimes) {
    File file = SPIFFS.open("/opTimes.txt", FILE_WRITE);
 
    if(!file){
        Serial.println("There was an error opening the file for writing");
        return false;
    }

    try {
      if(!file.print((String)wakeTimes.timezone + "\n")){throw;}
      for(int i = 0; i < _MAX_WAKES_DAY; i++) {
        if(wakeTimes.wakeupTimes[i].duration > 0) {
          if(!file.print((String)wakeTimes.wakeupTimes[i].opHour + ":" + (String)wakeTimes.wakeupTimes[i].opMinute + "/" + (String)wakeTimes.wakeupTimes[i].duration + "\n")){throw;}
        }
      }
    } catch(...){
      Serial.println("Problema ao salvar dados no arquivo");
      return false;
    }
 
    file.close();
    return true;
}

WiFiInsertion FilesReadFile() {
    WiFiInsertion toReturn;
    File file = SPIFFS.open("/opTimes.txt");
    if(!file){
        Serial.println("Failed to open file for reading");
        return toReturn;
    }
    int lineCounter = 0;
    String lineRead = "";
    while(file.available()){
      toReturn.success = true;
      char charRead = (char)file.read();
      if(charRead == '\n' || charRead == ':' || charRead == '/') {
        if(lineCounter == 0) {
          Serial.print("timezone ");
          toReturn.timezone = lineRead.toInt();
          Serial.println(toReturn.timezone);
          lineCounter++;
        } else {
          if(charRead == ':') {
            toReturn.wakeupTimes[lineCounter-1].opHour = lineRead.toInt();
            Serial.print("hora ");
            Serial.println(toReturn.wakeupTimes[lineCounter-1].opHour);
          } else if(charRead == '/') {
            toReturn.wakeupTimes[lineCounter-1].opMinute = lineRead.toInt();
            Serial.print("minuto ");
            Serial.println(toReturn.wakeupTimes[lineCounter-1].opMinute);
          } else {
            if(lineRead != "") {
              toReturn.wakeupTimes[lineCounter-1].duration = lineRead.toInt();
              Serial.print("duração ");
              Serial.println(toReturn.wakeupTimes[lineCounter-1].duration);
            }
            lineCounter++;
          }
        }
        lineRead = "";
      } else {
        lineRead += charRead;
      }
    }
    file.close();
    return toReturn;
}

void removeFile() {
  SPIFFS.remove("/opTimes.txt");
}
