/*
 * Arquivo para as funções do módulo GPS
 * 
 * Funções:
 * void GPSInitialize() - Inicializa o GPS
 * GPSPack GPSGetCoordinates() - Pega longitude, latitude e altitude do GPS
 * GPSTime GPSRetrieveTime() - Pega o horário pelo GPS
 */

GPSPack getCoordinates();
GPSTime getGPSTime();

void GPSInitialize() {
  ss.begin(GPSBaud, SERIAL_8N1, GPS_RX, GPS_TX);
  delay(2000);
  Serial.println("Inicializando GPS");
}

GPSPack GPSGetCoordinates() {
  GPSPack toReturn;
  int timeCounter = 0;
  while(timeCounter < 2*_MODULES_TIMEOUT) {
    while (ss.available() > 0) {
      while (gps.encode(ss.read())) {
        toReturn = getCoordinates();
       }
     }
    if(toReturn.latitudeNode != 0) {
      break;
    } else {
      timeCounter++;
      delay(500);
    }
  }
  return toReturn;
}

GPSTime GPSRetrieveTime() {
  GPSTime toReturn;
  int timeCounter = 0;
  while(timeCounter < 2*_MODULES_TIMEOUT) {
    while (ss.available() > 0) {
      if (gps.encode(ss.read())) {
        toReturn = getGPSTime();
      }
    }
    if(toReturn.yearGPS != 0) {
      break;
    } else {
      timeCounter++;
      delay(500);
    }
  }
  return toReturn;
}

GPSPack getCoordinates()
{
  GPSPack GPSObject;
  if (gps.location.isValid() && gps.altitude.isValid())
  {
    GPSObject.latitudeNode = gps.location.lat();
    GPSObject.longitudeNode = gps.location.lng();
    GPSObject.altitudeNode = gps.altitude.meters();
  }
  return GPSObject;
}

GPSTime getGPSTime() {
  //int offsetTimeZone = 3;
  GPSTime returnTime;
  if (gps.time.isValid() && gps.date.isValid()) {
    returnTime.yearGPS = gps.date.year();
    returnTime.monthGPS = gps.date.month();
    returnTime.dayGPS = gps.date.day();
    returnTime.hoursGPS = gps.time.hour();
    returnTime.minutesGPS = gps.time.minute();
    returnTime.secondsGPS = gps.time.second();
  }
  return returnTime;
}
