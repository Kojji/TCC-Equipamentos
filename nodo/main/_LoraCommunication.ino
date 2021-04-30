/*
 * Arquivo para as funções relacionadas a comunucação lora do gateway
 * 
 * Funções:
 * int LoraCommunicationStart() - Inicializa o Lora
 * int SendMessage(String) - Envia mensagem Lora
 * String buildMessage(char, String, char[], String) - Monta a mensagem no formato do pacote da rede
 * bool getLoRaData(LoraPack&, char[]) - Verifica se há mensagens endereçadas a este nodo e as lê
 * String LoraCommunicationInsertion(int) - Tenta inserir na rede, retorna o endereço do "pai" do nodo
 * bool LoraCommunicationSend(char[], String, String) - Manda mensagem em modo de operação do nodo
 *
*/

int LoraCommunicationStart(){
  int counter = 0;
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  while (!LoRa.begin(BAND) && counter < 10) {
    Serial.print(".");
    counter++;
    delay(500);
  }
  if (counter == 10) {
    Serial.println("Inicialização do Lora Falhou!");
    return 1; 
  }
  Serial.println("LoRa Inicializado!");
  delay(1000);
  return 0;
}

int SendMessage(String message) {
    LoRa.beginPacket();
    LoRa.print(message);
    LoRa.endPacket();
    Serial.print("");
    Serial.print("Enviando Mensagem: ");
    Serial.println(message);
    return 0;
}

String buildMessage(String code, String destAddress, String payload) {
  return code + destAddress + phyAddress + payload;
}

// upgrade - criar uma função para empacotar o recebimento de mensagens(uma camada antes do getLoRaData para usar no loop) e 
//    lidar com os diferentes códigos do lora

bool getLoRaData(LoraPack &package) {
  while (LoRa.available()) {
    String LoRaData = LoRa.readString();
    
    Serial.println("");
    Serial.print("Pacote Recebido via Lora: ");
    Serial.println(LoRaData);
    package.op = LoRaData.substring(0, CODE_SIZE);
    package.destAddress = LoRaData.substring(CODE_SIZE, CODE_SIZE + ADDRESS_SIZE);
    package.origAddress = LoRaData.substring(CODE_SIZE + ADDRESS_SIZE, CODE_SIZE + ADDRESS_SIZE + ADDRESS_SIZE);
    package.payload = LoRaData.substring(CODE_SIZE + ADDRESS_SIZE + ADDRESS_SIZE, LoRaData.length());
    if(package.destAddress != phyAddress || LoRaData.substring(0, CODE_SIZE) == CODE_INSERT_NODE) {
      package.op = "";
      package.destAddress = "";
      package.origAddress = "";
      package.payload = "";
      Serial.println("Ignorando Mensagem...");
      return false;
    } else {
      return true;
    }
  }
}

InsertionResponse LoraCommunicationInsertion(GPSPack coordinates) {
  InsertionResponse toReturn;
  LoraPack loraMessage;
  String code = CODE_INSERT_NODE;
  // corrigir - pegar a posição do nodo
  String payload = coordinates.getFormated();
  String message = buildMessage(code, "", payload );
  SendMessage(message);
  int counter = 0;
  while(counter < RESPONSE_TIMEOUT){
    Serial.print(".");
    int packetSize = LoRa.parsePacket();
    if(packetSize) {
      if(getLoRaData(loraMessage)) {
        if(loraMessage.op == CODE_RESPONSE) {
          toReturn = insertionPayloadProcess(loraMessage.payload);
          toReturn.fatherAddress = loraMessage.origAddress;
          break;
        } else {
          // upgrade - caso receba mensagem de operação sem endereço destino enquanto espera resposta de uma operação que este nodo realizou
        }
      }
    } else {
      delay(1000);
      counter ++;
    }
  }
  // upgrade - lógica para determinar qual equipamento será o "pai"
  // upgrade - troca de mensagens antes de definir quem é o "pai"
  
  return toReturn;
}

bool LoraCommunicationSend(String payload) {
  bool toReturn = false;
  LoraPack loraMessage;
  String code = CODE_DATA_SEND;
  String message = buildMessage(code, insertionData.fatherAddress, payload);
  SendMessage(message);
  int counter = 0;
  while(counter < RESPONSE_TIMEOUT){
    Serial.print(".");
    int packetSize = LoRa.parsePacket();
    if(packetSize) {
      if(getLoRaData(loraMessage)) {
        if(loraMessage.op == CODE_RESPONSE) {
          toReturn = true;
          break;
        } else {
          // upgrade - caso receba mensagem de operação sem endereço destino enquanto espera resposta de uma operação que este nodo realizou
        }
      }
    } else {
      delay(1000);
      counter ++;
    }
  }
  return toReturn;
}

InsertionResponse insertionPayloadProcess(String payload) {
  InsertionResponse toReturn;
  int timezoneSeparator = payload.indexOf('/');
  String timezone = payload.substring(0, timezoneSeparator);
  toReturn.timezone = timezone.toInt();
  String wakeTimesString = payload.substring(timezoneSeparator + 1, payload.length());
  Serial.println("Horários de operação: ");
  for(int i = 0; i < _MAX_WAKES_DAY; i++) {
    int timeSeparator = wakeTimesString.indexOf('/');
    String line = wakeTimesString.substring(0, timeSeparator);
    String beforeConvert = line.substring(0, line.indexOf(':'));
    toReturn.wakeupTimes[i].opHour = beforeConvert.toInt();
    
    line = line.substring(line.indexOf(':') + 1, timeSeparator);
    beforeConvert = line.substring(0, line.indexOf(':'));
    toReturn.wakeupTimes[i].opMinute = beforeConvert.toInt();
    
    beforeConvert = line.substring(line.indexOf(':') + 1, line.length());
    toReturn.wakeupTimes[i].duration = beforeConvert.toInt();

    Serial.print("Hora: " + (String)toReturn.wakeupTimes[i].opHour + ", ");
    Serial.print("Minuto: " + (String)toReturn.wakeupTimes[i].opMinute + ", ");
    Serial.println("Duração: " + (String)toReturn.wakeupTimes[i].duration);
    if(timeSeparator == wakeTimesString.length() - 1) {
      toReturn.success = true;
      break;
    }
    wakeTimesString = wakeTimesString.substring(timeSeparator + 1,wakeTimesString.length());
  }
  return toReturn;
}
