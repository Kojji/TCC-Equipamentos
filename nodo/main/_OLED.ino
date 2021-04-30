/*
 * Arquivo para as funções do OLED da placa
 * 
 * funções:
 * int OLEDStart() - configura e habilita o OLED
 * void OLEDShowId() - mostra id da placa
 * void OLEDShowStatus(int) - mostra status de inicialização
 */

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
  return 0;
}

void OLEDShowId() {
  display.setCursor(0,8);
  display.print(phyAddress);
}

void OLEDShowStatus(int messageNumber) {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  switch(messageNumber) {
    case 0:      
      OLEDShowId();
      display.setCursor(0,24);
      display.print("Tentando pegar a posicao de GPS.");
      display.display();
      break;
    case 1:
      OLEDShowId();
      display.setCursor(0,24);
      display.print("Nodo inserido na rede.");
      display.setCursor(0,32);
      display.print("desligando a tela em 3 seg.");
      display.display();
      delay(3000);
      display.clearDisplay();
      display.display();
      break;
    case 2:
      OLEDShowId();
      display.setCursor(0,24);
      display.print("Requisicao de inserção na rede não foi respondida.");
      display.display();
      break;
    case 3:
      OLEDShowId();
      display.setCursor(0,24);
      display.print("Timeout excedido, ainda tentando pegar a posicao GPS.");
      display.display();
      break;
    case 4:
      OLEDShowId();
      display.setCursor(0,24);
      display.print("Tentando se comunicar com o gateway.");
      display.display();
      break;
    default:
      break;  
  }
}

void OLEDShowSensorStatus(int messageNumber, String sensor) {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  switch(messageNumber) {
    case 0:      
      OLEDShowId();
      display.setCursor(0,24);
      display.print("Sensor de " + sensor + " funcionando.");
      display.display();
      break;
    case 1:
      OLEDShowId();
      display.setCursor(0,24);
      display.print("Falha no sensor de " + sensor + ", ou erro na instalação.");
      display.display();
      break;
    default:
      break;  
  }
}
