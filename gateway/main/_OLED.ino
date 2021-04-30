/*
 * Arquivo para as funções do OLED da placa
 * 
 * funções:
 * int OLEDStart() - configura e habilita o OLED
 * int OLEDShowId() - mostra id da placa
 * void OLEDShowStatus(int) - imprime determinada mensagem
 * void printMessage(String) - imprime mensagem
 */



Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

int OLEDStart() {
  //reseta display OLED por software
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);

  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) {
    Serial.println(F("SSD1306(display) Falhou na Inicialização!"));
    return 1;
  }
  display.clearDisplay();
  display.display();
  return 0;
}

int OLEDShowId() {
  display.setCursor(0,8);
  display.print(phyAddress);
  return 0;
}

void OLEDShowStatus(int messageNumber) {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  switch(messageNumber) {
    case 0:      
      OLEDShowId();
      display.setCursor(0,24);
      display.print("Trying to be inserted into the network.");
      display.display();
      break;
    case 1:
      OLEDShowId();
      display.setCursor(0,24);
      display.print("Inserted in the network.");
      display.setCursor(0,32);
      display.print("Turning this screen off in 3 sec.");
      display.display();
      delay(3000);
      display.clearDisplay();
      display.display();
      break;
    case 2:
      OLEDShowId();
      display.setCursor(0,24);
      display.print("Unable to connect with the server.");
      display.display();
      break;
    case 3:
      OLEDShowId();
      display.setCursor(0,24);
      display.print("Unable to connect with the WiFi.");
      display.display();
      break;
    case 4:
      OLEDShowId();
      display.setCursor(0,24);
      display.print("Falha ao tentar obter coordenadas.");
      display.display();
      break;
    case 5:
      OLEDShowId();
      display.setCursor(0,24);
      display.print("Iniciando a placa.");
      display.display();
      break;
    default:
      break;  
  }
}

void printMessage( String message ) {
  display.clearDisplay();
  display.setCursor(0,12);
  display.print(message);
  display.display();
}
