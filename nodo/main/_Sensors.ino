/*
 * Arquivo para as funções dos Sensores
 * 
 * Funções:
 * bool SensorsInitialize() - Inicializa os sensores
 * float SensorsGetTemp() - pega o valor da temperatura em Celsius
 */

bool SensorsInitialize() {
  sensors.begin();
}

float SensorsGetTemp(int index) {
  sensors.requestTemperatures(); 
  return sensors.getTempCByIndex(index);
}