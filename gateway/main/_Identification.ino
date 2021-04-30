/*
 * Arquivo para as funções de identificação do gateway e nodos da rede
 * 
 * funções
 * int IdentificationStoreId(char* phyAddress) - Pega o id do chip para utilizar como identificação da placa
 */


int IdentificationStoreId(char* phyAddress) {
  uint64_t chipid = ESP.getEfuseMac();
  uint16_t chip = (uint16_t)(chipid >> 32);
  snprintf(phyAddress, 23, "%04X%08X", chip, (uint32_t)chipid);
  return 0;
}
