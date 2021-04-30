// inforações de conexão internet

#define _WIFI_SSID "NET_2G26FF29"
#define _WIFI_PASS "6926FF29"

// #define _DATA_URL "https://api.agriv.com.br/api/v1/data-sensors"
#define _DATA_URL "https://webhook.site/59d411ee-6943-44db-878f-abae68a0b864"
// #define _INSERTION_URL "https://api.agriv.com.br/api/v1/sensors/register"
#define _INSERTION_URL "https://webhook.site/59d411ee-6943-44db-878f-abae68a0b864"

//define que pinagem usar, dependendo do modulo lora:
//1 - heltec Wifi LORA v2
#define _PIN_OUT 1

// número de mensagens que o gateway pode salvar sem enviar ao servidor
#define _MAX_MESSAGES 40

// frequencia de uplink UDP(em seg)
// #define _UP_LINK_FREQ 600

// quantidade máxima de horários de funcionamento do gateway por dia
#define _MAX_WAKES_DAY 5
