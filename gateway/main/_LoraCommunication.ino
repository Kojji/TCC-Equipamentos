/*
 * Arquivo para as funções relacionadas a comunucação lora do gateway
 * 
 * Funções:
 * int LoraCommunicationStart() - Inicializa o Lora 
 * String buildMessage(char, String, String) - Monta mensagem no formato do pacote
 * int LoraCommunicationSend(String) - Envia mensagem pelo Lora
 * bool answerMessage(LoraPack, WiFiInsertion) - Responde mensagem recebida pelo Lora
 * bool getLoRaData(LoraPack&, WiFiInsertion) - Lê mensagem recebida
 * bool LoraCommunicationHandleReceive(LoraPack&, WiFiInsertion) - Verifica se tem mensagem para ser lida
 * 
*/

#define BAND _BAND

#define CODE_SIZE _CODE_SIZE
#define ADDRESS_SIZE _ADDRESS_SIZE

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

String buildMessage(String code, String destAddress, String payload) {
  return code + destAddress + phyAddress + payload;
}

int LoraCommunicationSend(String message) {
    LoRa.beginPacket();
    LoRa.print(message);
    LoRa.endPacket();
    Serial.print("Respondendo mensagem recebida: ");
    Serial.println(message);
    return 0;
}

void answerMessage(LoraPack document, WiFiInsertion insertionData) {
  String code = CODE_RESPONSE;
  String message;
  if(document.op == CODE_INSERT_NODE) {
    String payload = insertionData.buildPayload();
    message = buildMessage(code, document.origAddress, payload);
  } else {
    message = buildMessage(code, document.origAddress, "");
  }
  LoraCommunicationSend(message);
}

bool getLoRaData(LoraPack &receivedPack) {
  yield();
  while (LoRa.available()) {
    String LoRaData = LoRa.readString();
    int pos1 = LoRaData.indexOf('/');
    int pos2 = LoRaData.indexOf('&');
    receivedPack.op = LoRaData.substring(0, CODE_SIZE);
    if(LoRaData.substring(0, CODE_SIZE) == CODE_INSERT_NODE) {
      receivedPack.origAddress = LoRaData.substring(CODE_SIZE, CODE_SIZE + ADDRESS_SIZE);
      receivedPack.payload = LoRaData.substring(CODE_SIZE + ADDRESS_SIZE, LoRaData.length());
    } else {
      receivedPack.destAddress = LoRaData.substring(CODE_SIZE, CODE_SIZE + ADDRESS_SIZE );
      if(receivedPack.destAddress != phyAddress) {
        receivedPack.op = "";
        receivedPack.destAddress = "";
        receivedPack.origAddress = "";
        receivedPack.payload = "";
        return false;
      } else {
        receivedPack.origAddress = LoRaData.substring(CODE_SIZE + ADDRESS_SIZE, CODE_SIZE + ADDRESS_SIZE + ADDRESS_SIZE);
        receivedPack.payload = LoRaData.substring(CODE_SIZE + ADDRESS_SIZE + ADDRESS_SIZE, LoRaData.length() );
        Serial.println("");
        Serial.print("Pacote Recebido via Lora: ");
        String rec = receivedPack.op + " - " + receivedPack.origAddress + " - " + receivedPack.destAddress + " - " + receivedPack.payload;  
        return true;
      }
    }
  }
}

bool LoraCommunicationHandleReceive(LoraPack &message, WiFiInsertion insertionData) {
  bool toReturn = false;
  int packetSize = LoRa.parsePacket();
  if (packetSize > 0) {
    toReturn = getLoRaData(message);
    if(message.op == CODE_INSERT_NODE) {
      int pos1 = message.payload.indexOf('/');
      String insertNodeJson = "{\"address\": \"" + message.origAddress + "\", \"type\": \"nodo\", \"lat\": " + message.payload.substring(0, pos1) + ", \"lng\": " + message.payload.substring(pos1+1, message.payload.length()) + " }";
      if(WiFiSendRequest(insertNodeJson, 1)) {
        answerMessage(message, insertionData);
      }
    } else {
      int pos1 = message.payload.indexOf('$');
      String firstHalf = message.payload.substring(0, pos1);
      int pos2 = firstHalf.indexOf('/');
      String secHalf = message.payload.substring(pos1 + 1, message.payload.length());
      int pos3 = secHalf.indexOf('/');
      int pos4 = secHalf.indexOf('#');
      String sendDataJson = "{\"address\": \"" + message.origAddress + "\", \"humiTwenty\": " + firstHalf.substring(0, pos2) + ", \"tempTwenty\": " + firstHalf.substring(pos2 + 1, firstHalf.length()) + ", \"humiFourty\": " + secHalf.substring(0, pos3) + ", \"tempFourty\": " + secHalf.substring(pos3 + 1, pos4) + ", \"pHTwenty\": " + secHalf.substring(pos4 + 1, secHalf.length()) + " }";
      if(WiFiSendRequest(sendDataJson, 0)) {
        answerMessage(message, insertionData);
      }
    }
  }
  return toReturn;
}
