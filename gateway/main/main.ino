
/*
 * Código de gateway com esp32
 *
 */

#include "settings.h"
#include "config.h"

#include <ArduinoJson.h>         // WiFiCommunication
#include <HTTPClient.h>          // WiFiCommunication
#include <WiFi.h>                // WiFiCommunication
#include <Wire.h>                // _OLED
#include <Adafruit_GFX.h>        // _OLED
#include <Adafruit_SSD1306.h>    // _OLED
#include <SPI.h>                 // _LoraCommunication
#include <LoRa.h>                // _LoraCommunication
#include <SPIFFS.h>              // _Files
#include <TimeLib.h>             // main
#include <ArduinoQueue.h>        // main
#include <TinyGPS++.h>             // _GPS
#include <HardwareSerial.h>        // _GPS

#include "classes.h"

#    if _PIN_OUT == 1
  #define LORA_SCK 5           // _Lora
  #define LORA_MISO 19         // _Lora
  #define LORA_MOSI 27         // _Lora
  #define LORA_SS 18           // _Lora
  #define LORA_RST 14          // _Lora
  #define LORA_DIO0 26         // _Lora
  #define OLED_SDA 4           // _OLED
  #define OLED_SCL 15          // _OLED
  #define OLED_RST 16          // _OLED
  #define SCREEN_WIDTH 128     // _OLED
  #define SCREEN_HEIGHT 64     // _OLED
  #define GPS_RX 12            // _GPS
  #define GPS_TX 13            // _GPS
#   endif

#define MAX_MESSAGES _MAX_MESSAGES

#define NODE_TEST false //define se o tempo de wake up será calculado, ou usado um padrão 
#define NODE_TEST_SLEEP_TIME 120 //tempo até wake up caso NODE_TEST == true
#define NODE_INSERTION_FAIL 1800//tempo em deepsleep se inserção na rede falhar
#define uS_TO_S_FACTOR 1000000ULL  //fator de conversão para segundos

// ---- _GPS ---- 
static const uint32_t GPSBaud = 9600;
TinyGPSPlus gps;
HardwareSerial ss(1);
// ----########---- 

void WifiConnect();
bool WiFiSendRequest(String json, int type);
int IdentificationStoreId(char phyAddress[]);
int OLEDStart();
void OLEDShowStatus(int messageNumber);
int LoraCommunicationStart();
bool LoraCommunicationHandleReceive(LoraPack &loraRead, WiFiInsertion insertionData);
void GPSInitialize();
GPSTime GPSRetrieveTime();
bool FilesInitializeFiles();
void printMessage(String message);
bool FilesWriteOperationTimes(WiFiInsertion wakeTimes);
WiFiInsertion FilesReadFile();
WiFiInsertion WiFiInsertionNetwork(GPSPack coordinates);

char phyAddress[23];               //id da placa

bool inNetwork = false;

void setup() 
{ 
  Serial.begin(115200);
  Serial.println("Iniciando a placa!");
  IdentificationStoreId(phyAddress);
  OLEDStart();
  OLEDShowStatus(5);
  if(!FilesInitializeFiles()) {
    Serial.println("Falha ao inicializar sistema de arquivos!");
  }
  Serial.print("Endereço físico:");
  Serial.println(phyAddress);
  LoraCommunicationStart();
  GPSInitialize();
  Serial.print("Conectando WiFi.");
  printMessage("Conectando WiFi");
  
  WifiConnect();
  Serial.println("WiFi conectado.");

  long timeToSleep = NODE_TEST_SLEEP_TIME;
  
  if(!getIfWakeUp()) {

    Serial.println("tentando pegar coordenadas GPS");
    OLEDShowStatus(4);
    GPSPack coordinates;
    int GPSTimeCounter = 0;
    while(1) {
      coordinates = GPSGetCoordinates();
      if(coordinates.latitude != 0) {
        break;
      } else {
        if(GPSTimeCounter > 2) {
          Serial.println("tentativas de obter dados GPS excedido. Resetando esp32.");
          ESP.restart();
        }
        GPSTimeCounter++;
        OLEDShowStatus(4);
      }
    }
    
    Serial.print("latitude:");
    Serial.println(coordinates.latitude,6);
    Serial.print("longitude:");
    Serial.println(coordinates.longitude,6);
    Serial.print("altitude:");
    Serial.println((String)coordinates.altitude);

    OLEDShowStatus(0);
    WiFiInsertion insertionData = WiFiInsertionNetwork(coordinates);
    if(insertionData.success) {
      FilesWriteOperationTimes(insertionData);
    } else {
      removeFile();
      timeToSleep = NODE_INSERTION_FAIL;
    }

    int counter = 0;
    while(counter < 30*60*1000) {
      
      LoraPack loraRead;

      LoraCommunicationHandleReceive(loraRead, insertionData);

      counter ++;
      delay(1);
    }

    // opera por 1h
    if(!NODE_TEST) {
      timeToSleep = insertionData.getTimeToSleep(GPSRetrieveTime());
    }
  } else {
    GPSTime wakeupTime;
    wakeupTime = GPSRetrieveTime();
    Serial.println(wakeupTime.getFormated());

    WiFiInsertion insertionData = FilesReadFile();
    if(!insertionData.success) {
      ESP.restart();
    }
    Serial.println(insertionData.buildPayload());

    while(insertionData.checkOperation(GPSRetrieveTime())) {
      LoraPack loraRead;

      LoraCommunicationHandleReceive(loraRead, insertionData);   
    }

    if(!NODE_TEST) {
      timeToSleep = insertionData.getTimeToSleep(GPSRetrieveTime());
    }
  }

  esp_sleep_enable_timer_wakeup(timeToSleep * uS_TO_S_FACTOR);
  Serial.println("Operação Completa, Nodo entrará em deep sleep por " + String(timeToSleep/60) +
  " minutos");
  Serial.println();
  esp_deep_sleep_start();
}

 
void loop() {}

bool getIfWakeUp(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();
  bool isSetup = true; 

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Nodo acordou por timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.println("Nodo operando após reset.");isSetup = false; break;
  }

  // retorna true se acordou de deepsleep, false se foi inicializado 1ª vez
  return isSetup;
}
