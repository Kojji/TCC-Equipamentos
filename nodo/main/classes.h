/*
 * Arquivo para definição de classes
 * 
 * Classes:
 * class LoraPack - classe com o formato do pacote de Lora que é enviado na rede
 * class GPSPack - classe com as coordenadas geográficas
 * class GPSTime - classe com os valores que consistem no horário
 * class OperationHour - classe com os atributos que definem um horário de operação
 * class InsertionResponse - classe que armazena os dados referentes à rede que o nodo foi inserido
 */

class LoraPack {
  public:
    String op = "";
    String destAddress = "";
    String origAddress = "";
    String payload = "";
};

class GPSPack {
  public:
    double latitudeNode = 0;
    double longitudeNode = 0;
    double altitudeNode = 0;

    String getFormated() {
      return String(this->latitudeNode,6) + "/" + String(this->longitudeNode,6) + "/" + (String)this->altitudeNode;
    }
};

class GPSTime {
  public:
    int secondsGPS = 0;
    int minutesGPS = 0;
    int hoursGPS = 0;
    int dayGPS = 0;
    int monthGPS = 0;
    int yearGPS = 0;

    String getFormated() {
      String toReturn = "horário: " + (String)hoursGPS + ":" + (String)minutesGPS + ":" + (String)secondsGPS;
      toReturn += "\nData: " + (String)dayGPS + "/" + (String)monthGPS + "/" + (String)yearGPS;
      return toReturn;
    }

};

class OperationHour {
  public:
    int opHour = 0;
    int opMinute = 0;
    int duration = 0;
};

class InsertionResponse {
  public:
    OperationHour wakeupTimes[_MAX_WAKES_DAY];
    bool success = false;
    int timezone;
    String fatherAddress;
    
    long getTimeToSleep(GPSTime timeNow) {
      //ajusta pela timezone
      int timezoned = timeNow.hoursGPS + this->timezone;
      if(timezoned >= 24) {
        timezoned = timezoned - 24;
      } else if(timezoned < 0) {
        timezoned = timezoned + 24;
      }

      //identifica qual o próximo horário para acordar
      int nextIndex = 0;
      for(int i = 0; i < _MAX_WAKES_DAY; i++) {
        if(this->wakeupTimes[i].duration > 0){
          if(timezoned < this->wakeupTimes[i].opHour || (timezoned == this->wakeupTimes[i].opHour && timeNow.minutesGPS < this->wakeupTimes[i].opMinute)) {
            nextIndex = i;
            break;
          }
        } else {
          break;
        }
      }
      
      //calcular diferença de tempo em segundos
      int hourDiff = this->wakeupTimes[nextIndex].opHour - timezoned;
      if(hourDiff < 0) { hourDiff = 24 + hourDiff; }
      int minuteDiff = this->wakeupTimes[nextIndex].opMinute - timeNow.minutesGPS;
      Serial.print("hourDiff: " + (String)hourDiff + " minDiff: " + (String)minuteDiff);
      // número aleatório para acordar o nodo sem que seja nos primeiros 5 e 10 ultimos minutos de op do gateway
      long randomNumber = random(5, this->wakeupTimes[nextIndex].duration * 60 - 10);
      Serial.print("Número randomico ");
      Serial.println(randomNumber);
      return (hourDiff * 3600) + (minuteDiff * 60) + (randomNumber * 60);
    }

};
