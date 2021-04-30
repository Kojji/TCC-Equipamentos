/*
 * Arquivo para as funções de leitura de arquivos
 * 
 * funções:
 * bool FilesInitializeFiles() - inicializa o sistema de arquivos
 * bool FilesWriteInsertionFile - escreve os dados da rede no arquivo
 * InsertionResponse FilesReadInsertionFile() - le os dados da rede do arquivo
 */

bool FilesInitializeFiles() {
  if(!SPIFFS.begin(true)){
    return false;
  }
  return true;
}

bool FilesWriteInserionFile(InsertionResponse insertionData) {
    File file = SPIFFS.open("/inserted.txt", FILE_WRITE);
 
    if(!file){
        Serial.println("There was an error opening the file for writing");
        return false;
    }

    try {
      if(!file.print(insertionData.fatherAddress + "\n")){throw;}
      if(!file.print((String)insertionData.timezone + "\n")){throw;}
      for(int i = 0; i < _MAX_WAKES_DAY; i++) {
        if(insertionData.wakeupTimes[i].duration > 0) {
          if(!file.print((String)insertionData.wakeupTimes[i].opHour + ":")){throw;}
          if(!file.print((String)insertionData.wakeupTimes[i].opMinute + "/")){throw;}
          if(!file.print((String)insertionData.wakeupTimes[i].duration + "\n")){throw;}
        }
      }
    } catch(...){
      Serial.println("Problema ao salvar dados no arquivo");
      return false;
    }
 
    file.close();
    return true;
}

void removeFile() {
  SPIFFS.remove("/inserted.txt");
}

InsertionResponse FilesReadInsertionFile() {
    InsertionResponse toReturn; 
    File file = SPIFFS.open("/inserted.txt");
 
    if(!file){
        Serial.println("Failed to open file for reading");
        return toReturn;
    }
    
    int lineCounter = 0;
    String lineRead = "";
    while(file.available()){
      char charRead = (char)file.read();
      if(charRead == '\n' || charRead == ':' || charRead == '/') {
        if(lineCounter == 0) {
          //Serial.print("fatherAddress ");
          toReturn.fatherAddress = lineRead;
          //Serial.println(toReturn.fatherAddress);
          lineCounter++;
        } else if(lineCounter == 1) {
          //Serial.print("timezone ");
          toReturn.timezone = lineRead.toInt();
          //Serial.println(toReturn.timezone);
          lineCounter++;
        } else {
          if(charRead == ':') {
            toReturn.wakeupTimes[lineCounter-1].opHour = lineRead.toInt();
            //Serial.print("hora ");
            //Serial.println(toReturn.wakeupTimes[lineCounter-1].opHour);
          } else if(charRead == '/') {
            toReturn.wakeupTimes[lineCounter-1].opMinute = lineRead.toInt();
            //Serial.print("minuto ");
            //Serial.println(toReturn.wakeupTimes[lineCounter-1].opMinute);
          } else {
            if(lineRead != "") {
              toReturn.wakeupTimes[lineCounter-1].duration = lineRead.toInt();
              //Serial.print("duração ");
              //Serial.println(toReturn.wakeupTimes[lineCounter-1].duration);
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
