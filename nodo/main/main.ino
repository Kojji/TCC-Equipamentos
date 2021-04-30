/*
 * Código de nodo com esp32
 */

#include "settings.h"
#include "classes.h"

#include <LoRa.h>                  // _Lora
#include <SPI.h>                   // _Lora
#include <Wire.h>                  // _OLED
#include <Adafruit_GFX.h>          // _OLED
#include <Adafruit_SSD1306.h>      // _OLED
#include <SPIFFS.h>                // _Files
#include <TinyGPS++.h>             // _GPS
#include <HardwareSerial.h>        // _GPS
#include <OneWire.h>               // _Sensors
#include <DallasTemperature.h>     // _Sensors

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
  #define SENSORS_PIN_PH 36  // _Sensors
  #define SENSORS_PIN_HUMI1 38  // _Sensors
  #define SENSORS_PIN_HUMI2 39  // _Sensors
  #define SENSORS_PIN_TEMP 17  // _Sensors
#   endif

#define NODE_TEST false //define se o tempo de wake up será calculado, ou usado um padrão 
#define NODE_TEST_SLEEP_TIME 120 //tempo até wake up caso NODE_TEST == true
#define NODE_INSERTION_FAIL 1800//tempo em deepsleep se inserção na rede falhar
#define uS_TO_S_FACTOR 1000000ULL  //fator de conversão para segundos

// ---- _LoraCommunication ----
#define BAND _BAND
#define CODE_SIZE _CODE_SIZE
#define ADDRESS_SIZE _ADDRESS_SIZE
#define RESPONSE_TIMEOUT _RESPONSE_TIMEOUT
// ----########---- 

// ---- Variaveis globais ---- 
char phyAddress[23];
InsertionResponse insertionData;
// ----########---- 

// ---- _OLED ----
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);
// ----########---- 

// ---- _GPS ---- 
static const uint32_t GPSBaud = 9600;
TinyGPSPlus gps;
HardwareSerial ss(1);
// ----########---- 

// ---- _Sensors ---- 
OneWire oneWire(SENSORS_PIN_TEMP);
DallasTemperature sensors(&oneWire);
// ----########---- 

int LoraCommunicationStart();
bool LoraCommunicationSend(String payload);
InsertionResponse LoraCommunicationInsertion(GPSPack coordinates);
int IdentificationStoreId(char phyAddress[]);
bool FilesInitializeFiles();
bool FilesWriteInserionFile(InsertionResponse fatherAddress);
void removeFile();
InsertionResponse FilesReadInsertionFile();
void GPSInitialize();
GPSPack GPSGetCoordinates();
GPSTime GPSRetrieveTime();
int OLEDStart();
void OLEDShowStatus(int messageNumber);
void OLEDShowSensorStatus(int messageNumber, String sensor);
bool SensorsInitialize();
float SensorsGetTemp(int index);
bool getIfWakeUp();


void setup() 
{ 
  pinMode (SENSORS_PIN_HUMI1,INPUT);
  pinMode (SENSORS_PIN_HUMI2,INPUT);
  pinMode (SENSORS_PIN_PH,INPUT);
  
  Serial.begin(115200);
  Serial.println("Iniciando a placa!");
  
  GPSInitialize();
  if(!FilesInitializeFiles()) {
    Serial.println("Falha ao inicializar sistema de arquivos!");
  }
  IdentificationStoreId(phyAddress);
  SensorsInitialize();
  Serial.print("Endereço físico:");
  Serial.println(phyAddress);
  LoraCommunicationStart();
  
  long timeToSleep = NODE_TEST_SLEEP_TIME;
  
  if(!getIfWakeUp()) {
    // ligado pela primeira vez
    
    OLEDStart();
    OLEDShowStatus(0);

    //teste do sensor de umidade a 20 cm
    float moisture1 = analogRead(SENSORS_PIN_HUMI1);
    Serial.println("moisture1: " + (String)moisture1);
    if(moisture1 != 0 && moisture1 != 4095) {
      OLEDShowSensorStatus(0, "umidade1");
    } else {
      OLEDShowSensorStatus(1, "umidade1");
    }
    delay(3000);

    //teste do sensor de umidade a 40 cm
    float moisture2 = analogRead(SENSORS_PIN_HUMI2);
    Serial.println("moisture2: " + (String)moisture2);
    if(moisture2 != 0 && moisture2 != 4095) {
      OLEDShowSensorStatus(0, "umidade2");
    } else {
      OLEDShowSensorStatus(1, "umidade2");
    }
    delay(3000);

    //teste do sensor de pH a 20 cm
    float ph1 = analogRead(SENSORS_PIN_PH);
    Serial.println("ph1: " + (String)ph1);
    if(ph1 != 0 && ph1 != 4095) {
      OLEDShowSensorStatus(0, "pH");
    } else {
      OLEDShowSensorStatus(1, "pH");
    }
    delay(3000);

    //teste do sensor de temperatura a 20 cm
    float temp1 = SensorsGetTemp(0);
    Serial.println("temp1: " + (String)temp1);
    if(temp1 != -127){
      OLEDShowSensorStatus(0, "temperatura1");
    } else {
      OLEDShowSensorStatus(1, "temperatura1");
    }
    delay(3000);

    //teste do sensor de temperatura a 40 cm
    float temp2 = SensorsGetTemp(1);
    Serial.println("temp2: " + (String)temp2);
    if(temp2 != -127){
      OLEDShowSensorStatus(0, "temperatura2");
    } else {
      OLEDShowSensorStatus(1, "temperatura2");
    }
    delay(3000);
    
    Serial.println("tentando pegar coordenadas GPS");
    GPSPack coordinates;
    int GPSTimeCounter = 0;
    while(1) {
      coordinates = GPSGetCoordinates();
      if(coordinates.latitudeNode != 0) {
        break;
      } else {
        if(GPSTimeCounter > 2) {
          Serial.println("tentativas de obter dados GPS excedido. Resetando esp32.");
        }
        GPSTimeCounter++;
        OLEDShowStatus(3);
      }
    }
    
    Serial.print("latitude:");
    Serial.println(coordinates.latitudeNode,6);
    Serial.print("longitude:");
    Serial.println(coordinates.longitudeNode,6);
    Serial.print("altitude:");
    Serial.println((String)coordinates.altitudeNode);
    // Inserção na rede
    bool inNetwork = false;
    OLEDShowStatus(4);
    int timeCounter = 0;
    while(!inNetwork) {
      //timeout + 45seg = tempo de loop
      //timeout:15; 1 loops = 1 min
      insertionData = LoraCommunicationInsertion(coordinates);
      if(insertionData.fatherAddress.length() > 0) {
        if(!FilesWriteInserionFile(insertionData)){
          Serial.println("Falha ao escrever arquivo!");
        }
        inNetwork = true;
        OLEDShowStatus(1);
      } else {
        OLEDShowStatus(2);
        delay(30000);
        timeCounter++;
        if(timeCounter >= 10) {
          removeFile();
          timeToSleep = NODE_INSERTION_FAIL;
          break;
        }
      }
    }
    if(!NODE_TEST) {
      timeToSleep = insertionData.getTimeToSleep(GPSRetrieveTime());
    }
  } else {
    int failures = 0;
    while(failures < 3) {
      // acordou de deepsleep
      GPSTime wakeupTime;
      wakeupTime = GPSRetrieveTime();
      Serial.println(wakeupTime.getFormated());
      
      Serial.print("Endereço físico:");
      Serial.println(phyAddress);
  
      insertionData = FilesReadInsertionFile();
      if(insertionData.fatherAddress.length() == 0) {
        ESP.restart();
      }
      Serial.print("Endereço pai:");
      Serial.println(insertionData.fatherAddress);
      String LoRaMessage = "";

      //sensor humidade
      float moisture1 = analogRead(SENSORS_PIN_HUMI1);
      Serial.println("moisture1: " + (String)moisture1);
      LoRaMessage += (String)moisture1 + "/";

      float temp1 = SensorsGetTemp(0);
      Serial.println("temperature1: " + (String)temp1);
      LoRaMessage += (String)temp1 + "$";
      
      float moisture2 = analogRead(SENSORS_PIN_HUMI2);
      Serial.println("moisture2: " + (String)moisture2);
      LoRaMessage += (String)moisture2 + "/";

      float temp2 = SensorsGetTemp(1);
      Serial.println("temperature2: " + (String)temp2);
      LoRaMessage += (String)temp2 + "#";

      float ph1 = analogRead(SENSORS_PIN_PH);
      Serial.println("ph1: " + (String)ph1);
      LoRaMessage += (String)ph1;
      
      // montar loop da quantidade de tentativas
      int tries = 0;
      bool success = false;
      while(tries < 3) {
        yield();
        if(LoraCommunicationSend(LoRaMessage)) {
          success = true;
          break;
        } else {
          Serial.println("Falha ao enviar mensagem.");
          tries ++;
          delay(5000);
        }
      }
      if(success) {
        if(!NODE_TEST) {
          timeToSleep = insertionData.getTimeToSleep(wakeupTime);
        }
        break;
      } else if(failures == 2){
        ESP.restart();
        break;
      } else {
        failures++;
        delay(90000);
      }
    }
  }

  esp_sleep_enable_timer_wakeup(timeToSleep * uS_TO_S_FACTOR);
  Serial.println("Operação Completa, Nodo entrará em deep sleep por " + String(timeToSleep/60) +
  " minutos");
  Serial.println();
  esp_deep_sleep_start();
}

void loop() 
{
}

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
