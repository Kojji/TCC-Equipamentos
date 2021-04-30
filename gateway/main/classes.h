/*
 * Arquivo para definição de classes
 * 
 * Classes:
 * class LoraPack - classe com o formato do pacote de Lora que é enviado na rede
 */

class GPSPack {
  public:
    double latitude = 0;
    double longitude = 0;
    double altitude = 0;

    String getFormated() {
      return String(this->latitude,6) + "/" + String(this->longitude,6) + "/" + (String)this->altitude;
    }
};

class LoraPack {
  public:
    String op = "";
    String destAddress = ""; 
    String origAddress = "";
    String payload = "";
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

class WiFiInsertion {
  public:
    OperationHour wakeupTimes[_MAX_WAKES_DAY];
    bool success = false;
    int timezone = 0;

    String buildPayload() {
      String toReturn = "";
      toReturn += (String)this->timezone + "/";
      for(int i = 0; i< _MAX_WAKES_DAY ; i++) {
        if(this->wakeupTimes[i].duration > 0) {
          toReturn += (String)this->wakeupTimes[i].opHour + ":" + (String)this->wakeupTimes[i].opMinute + ":" + this->wakeupTimes[i].duration + "/";
        }
      }
      return toReturn;
    }

    bool checkOperation (GPSTime timeNow) {
      //ajusta pela timezone
      int timezoned = timeNow.hoursGPS + this->timezone;
      if(timezoned >= 24) {
        timezoned = timezoned - 24;
      } else if(timezoned < 0) {
        timezoned = timezoned + 24;
      }

      //identifica qual o ciclo em  que está
      int nowTimeConverted = (timezoned*60)+timeNow.minutesGPS;

      int nextIndex = 0;
      for(int i = 0; i < _MAX_WAKES_DAY; i++) {
        if(this->wakeupTimes[i].duration > 0){
          if(
            ((this->wakeupTimes[i].opHour*60)+this->wakeupTimes[i].opMinute) < nowTimeConverted &&
            (((this->wakeupTimes[i].opHour + this->wakeupTimes[i].duration)*60)+this->wakeupTimes[i].opMinute) > nowTimeConverted
          ) {
            nextIndex = i;
            break;
          }
        } else {
          break;
        }
      }
      
      if(nextIndex > 0) return true;
      return false;
    }

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
      return (hourDiff * 3600) + (minuteDiff * 60);
    }
};
