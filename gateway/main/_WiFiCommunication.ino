/*
 * Arquivo para funções de comunicação UDP(uplink e downlink)
 * 
 * Funções:
 * void WifiConnect() - Conecta no WiFi
 * WiFiInsertion WiFiInsertionNetwork(GPSPack) - Envia a requisição de inserção na rede
 * bool WiFiSendRequest(String, int) - Envia uma requisição http
*/

void WifiConnect()  {
  WiFi.begin(_WIFI_SSID, _WIFI_PASS);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
}

WiFiInsertion WiFiInsertionNetwork(GPSPack coordinates) {
  StaticJsonDocument<1024> doc, response, cycles;
  String httpRequestData;
  doc["type"] = "gateway";
  doc["address"] = (String)phyAddress;
  doc["ltd"] = String(coordinates.latitude,6);
  doc["lng"] = String(coordinates.longitude,6);
  serializeJson(doc, httpRequestData);
  WiFiInsertion toReturn;
  if(WiFi.status()== WL_CONNECTED){
    HTTPClient http;
    String url = _INSERTION_URL;
    http.begin(url);

    http.addHeader("Content-Type", "application/json");
    // String httpRequestData = "{\"address\": "  + (String)phyAddress + ", \"type\": \"gateway\"}";
    Serial.println(httpRequestData);
    int httpResponseCode = http.POST(httpRequestData);
    DeserializationError error = deserializeJson(response, http.getString());

    if (error) {
      Serial.println("Erro na Conversão da resposta de inserção em JSON");
      return toReturn;
    }

    Serial.println();
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    Serial.println("Conteúdo:");
    serializeJson(response, Serial);
    Serial.println("");
    http.end();

    if(httpResponseCode == 200) {
      if((bool)response["success"]) {
        for(int i=0;i<response["cycles"].size();i++) {
          toReturn.wakeupTimes[i].opHour = response["cycles"][i]["hour"];
          toReturn.wakeupTimes[i].opMinute = response["cycles"][i]["minute"];
          toReturn.wakeupTimes[i].duration = response["cycles"][i]["duration"];
        }
        toReturn.success = true;
      }
      return toReturn;
    }
  }
  return toReturn;
}

bool WiFiSendRequest(String json, int type) {
  if(WiFi.status()== WL_CONNECTED){
    HTTPClient http;
    String url = _INSERTION_URL;
    if(type == 0) {
      url = _DATA_URL;
    }
    http.begin(url);

    http.addHeader("Content-Type", "application/json");
    String httpRequestData = json;
    Serial.println(json);
    int httpResponseCode = http.POST(httpRequestData);
    Serial.println(http.getString());
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    http.end();
    if(httpResponseCode == 200) {
      return true;
    } else {
      return false;
    } 
  } else {
    return false;
  }
}
