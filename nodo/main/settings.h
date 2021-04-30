//define que pinagem usar, dependendo do modulo lora:
//1 - heltec Wifi LORA v2
#define _PIN_OUT 1

//433E6 for Asia
//866E6 for Europe
//915E6 for North America
#define _BAND 915E6

// definições do formato de pacote LORA
#define _CODE_SIZE 1
#define _ADDRESS_SIZE 12

// tempo de timeout (em segundos) da mensagem lora
#define _RESPONSE_TIMEOUT 15

// tempo de timeout (em segundos) da mensagem lora
#define _MODULES_TIMEOUT 120

// códigos usados na rede
#define CODE_INSERT_NODE "N" // código inserção de nodo na rede  
#define CODE_INSERT_GATE "G" // código inserção de gateway da rede
#define CODE_DATA_SEND "D" // código envio de dados em payload(usado para transitar dados dos sensores)
#define CODE_RESPONSE "R" // código resposta a uma mensagem

// sensores acoplados ao nodo
#define SENSOR_HUMIDADE true
#define SENSOR_PH true
#define SENSOR_LUMINOSIDADE true
#define SENSOR_TEMPERATURA true

// quantidade máxima de horários de funcionamento do gateway por dia
#define _MAX_WAKES_DAY 5
