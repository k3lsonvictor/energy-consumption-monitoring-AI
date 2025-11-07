// ==========================================
// FUNÇÕES EEPROM
// ==========================================

// Salvar energia acumulada na EEPROM
void saveEnergyToEEPROM() {
  EEPROM.put(0, totalEnergyWh);
  EEPROM.commit();
}

// Carregar energia acumulada da EEPROM
void loadEnergyFromEEPROM() {
  EEPROM.get(0, totalEnergyWh);
  if (isnan(totalEnergyWh) || totalEnergyWh < 0) {
    totalEnergyWh = 0.0; // Reset se valor inválido
  }
  Serial.print("Energia carregada da EEPROM: ");
  Serial.print(totalEnergyWh, 6);
  Serial.println(" Wh");
}

// Resetar energia acumulada
void resetEnergy() {
  totalEnergyWh = 0.0;
  saveEnergyToEEPROM();
  Serial.println("Energia resetada!");
}

