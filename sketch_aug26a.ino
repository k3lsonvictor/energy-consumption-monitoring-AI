// ==========================================
// ARQUIVO PRINCIPAL - MEDIÇÃO DE ENERGIA
// ==========================================

#include <WiFi.h>
#include <HTTPClient.h>
#include <EEPROM.h>
#include "config.h"

// ==========================================
// VARIÁVEIS GLOBAIS
// ==========================================

// Offsets dos sensores
float voltageOffsetCurrent = 0;              // Offset do sensor de corrente (ACS712)
float voltageOffsetVoltage = 2.5;            // Offset do sensor de tensão (ZMPT101B)

// Energia e controle
float totalEnergyWh = 0.0;                   // Energia acumulada total
int saveCounter = 0;                         // Contador para salvar periodicamente
unsigned long lastSaveTime = 0;               // Última vez que salvou no banco

// Controles
bool autoOffsetAdjust = false;               // Desabilitar ajuste automático por padrão
bool useRealVoltage = true;                  // Usar tensão real ou fixa

// ==========================================
// SETUP
// ==========================================

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n═══════════════════════════════════");
  Serial.println("  ⚡ SISTEMA DE MEDIÇÃO DE ENERGIA");
  Serial.println("═══════════════════════════════════\n");
  
  // Configurar ADC para medição de tensão
  analogReadResolution(12); // Resolução 12 bits
  analogSetPinAttenuation(voltagePin, ADC_11db); // Até ~3.6V (para ZMPT101B)
  
  // Inicializar EEPROM
  EEPROM.begin(512);
  
  // Carregar energia acumulada da EEPROM
  loadEnergyFromEEPROM();
  
  // Configurar Wi-Fi (tenta conectar ou inicia modo AP)
  loadWiFiCredentials();
  bool wifiConnected = connectToWiFi();
  
  if (!wifiConnected) {
    // Se não conseguiu conectar, inicia modo Access Point
    startAccessPoint();
    Serial.println("\n⚠️  Sistema em modo configuração.");
    Serial.println("   Configure o Wi-Fi via web antes de continuar.");
    Serial.println("   O sistema continuará funcionando, mas não salvará no banco.\n");
  }
  
  // Calibrar offset do sensor de corrente
  calibrateOffset();
  
  // Aviso sobre medição de tensão
  if (!useRealVoltage) {
    Serial.println("\n⚠️  ATENÇÃO: Medição de tensão real DESABILITADA");
    Serial.println("   Sistema usando tensão fixa de 220V");
    Serial.println("   Execute 'voltage' para habilitar medição real (ZMPT101B)");
  } else {
    Serial.println("\n✅ Medição de tensão real HABILITADA (ZMPT101B)");
  }
  
  // Mostrar comandos disponíveis
  printHelp();
}

// ==========================================
// LOOP PRINCIPAL
// ==========================================

void loop() {
  unsigned long currentTime = millis();
  
  // Processar servidor web (se estiver em modo AP para configuração)
  if (WiFi.status() != WL_CONNECTED && WiFi.getMode() == WIFI_AP) {
    handleWiFiManager();
  }
  
  // Processar comandos Serial
  processSerialCommands();
  
  // Medir sensores
  float rmsCurrent = 0;
  float rmsVoltage = 0;
  measureSensors(rmsCurrent, rmsVoltage);
  
  // Verificar e ajustar offset (se necessário)
  checkAndAdjustOffset(rmsCurrent);
  
  // Calcular potência e energia
  float powerWatts = rmsCurrent * rmsVoltage * powerFactor;
  float energyWh = powerWatts * (5.0 / 3600.0); // energia em Wh (5 segundos = 5/3600 horas)
  float durationMin = 5.0 / 60.0; // duração em minutos (5 segundos = 5/60 minutos)
  
  // Acumular energia total APENAS se houver leituras válidas
  int rawValue = analogRead(sensorPin);
  if (rawValue > 0) {
    totalEnergyWh += energyWh;
    saveCounter++;
  } else {
    // Se ADC = 0, não acumular e avisar
    static unsigned long lastErrorTime = 0;
    if (millis() - lastErrorTime > 10000) {
      Serial.println("❌ ERRO CRÍTICO: ADC retornando 0! Sensor desconectado!");
      lastErrorTime = millis();
    }
    delay(5000);
    return; // Pular esta iteração
  }
  
  // Debug: mostrar valores atuais
  float voltage_adc = (rawValue * VCC_ADC) / adcResolution;
  float voltage_sensor = voltage_adc * divisorFactor;
  float currentInstant = (voltage_sensor - voltageOffsetCurrent) / sensitivity;
  
  // Calcular tempo restante para próximo salvamento
  unsigned long timeToNextSave = saveIntervalMs - (currentTime - lastSaveTime);
  
  // Mostrar valores no Serial Monitor
  Serial.print("ADC: ");
  Serial.print(rawValue);
  if (rawValue == 0) {
    Serial.print(" (ERRO!)");
  }
  Serial.print(", V_ADC: ");
  Serial.print(voltage_adc, 3);
  Serial.print(" V, V_Sensor: ");
  Serial.print(voltage_sensor, 3);
  Serial.print(" V, Offset: ");
  Serial.print(voltageOffsetCurrent, 3);
  Serial.print(" V, Corrente instantânea: ");
  Serial.print(currentInstant, 4);
  Serial.print(" A, Corrente RMS: ");
  Serial.print(rmsCurrent, 3);
  Serial.print(" A, Tensão RMS: ");
  Serial.print(rmsVoltage, 1);
  if (useRealVoltage) {
    Serial.print(" V (REAL)");
  } else {
    Serial.print(" V (FIXA)");
  }
  Serial.print(", Potência: ");
  Serial.print(powerWatts, 2);
  Serial.print(" W, Energia (5s): ");
  Serial.print(energyWh, 6);
  Serial.print(" Wh, Total acumulado: ");
  Serial.print(totalEnergyWh, 6);
  Serial.print(" Wh, Próximo save em: ");
  Serial.print(timeToNextSave / 1000);
  Serial.println(" s");
  
  // Salvar na EEPROM a cada 10 leituras (50 segundos)
  if (saveCounter >= 10) {
    saveEnergyToEEPROM();
    saveCounter = 0;
    Serial.println("Energia salva na EEPROM!");
  }
  
  // Enviar dados para o servidor a cada 10 minutos
  if (WiFi.status() == WL_CONNECTED) {
    if (lastSaveTime == 0 || (currentTime - lastSaveTime >= saveIntervalMs)) {
      sendDataToServer(totalEnergyWh, 10.0);
      lastSaveTime = currentTime;
    }
  } else {
    // Se WiFi desconectado, mostrar aviso ocasionalmente
    static unsigned long lastWifiWarning = 0;
    if (currentTime - lastWifiWarning > 30000) { // A cada 30 segundos
      Serial.println("⚠️  WiFi desconectado! Não é possível salvar no banco.");
      lastWifiWarning = currentTime;
    }
  }
  
  delay(5000); // medir a cada 5 segundos
}
