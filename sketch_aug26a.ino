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
float voltageOffsetCurrent = 2.5;            // Offset do sensor de corrente (ACS712) - 2.5V quando não há corrente
float voltageOffsetVoltage = 2.5;            // Offset do sensor de tensão (ZMPT101B)

// Energia e controle
float totalEnergyWh = 0.0;                   // Energia acumulada total
int saveCounter = 0;                         // Contador para salvar periodicamente
unsigned long lastSaveTime = 0;               // Última vez que salvou no banco

// Controles
bool autoOffsetAdjust = false;               // Desabilitar ajuste automático por padrão
bool useRealVoltage = true;                  // Usar tensão real ou fixa

// Acumuladores para cálculo de potência ativa por amostragem instantânea
float sumV2 = 0.0;                           // Soma de v² (tensão instantânea ao quadrado)
float sumI2 = 0.0;                           // Soma de i² (corrente instantânea ao quadrado)
float sumP = 0.0;                            // Soma de v * i (potência instantânea)
double sumCurrentADC = 0.0;                  // Soma dos valores ADC de corrente (para recalibração de offset)
unsigned long samples = 0;                   // Contador de amostras válidas
unsigned long lastCalculationTime = 0;       // Última vez que calculou valores RMS e potência
const unsigned long calculationIntervalMs = 1000; // Intervalo para cálculo (1 segundo)
const float noLoadThreshold = 0.15;          // Limiar para detectar ausência de carga (150mA)

// ==========================================
// DECLARAÇÕES DE FUNÇÕES (PROTOTYPES)
// ==========================================

// Funções de calibração
void calibrateOffset(int numSamples = 1000);
void calibrateVoltageOffset(int numSamples = 1000);

// Funções de sensores
void measureSensors(float& rmsCurrent, float& rmsVoltage);
void checkAndAdjustOffset(float rmsCurrent);
void sampleInstantaneousValues();            // Amostragem instantânea de tensão e corrente
void calculatePowerValues(float& rmsVoltage, float& rmsCurrent, float& realPower, float& apparentPower, float& powerFactor); // Calcular valores RMS e potência

// Funções EEPROM
void loadEnergyFromEEPROM();
void saveEnergyToEEPROM();
void resetEnergy();

// Funções WiFi
void loadWiFiCredentials();
bool connectToWiFi();
void startAccessPoint();
void handleWiFiManager();

// Funções HTTP
void sendDataToServer(float energy, float duration, float realPower = 0, float apparentPower = 0, float powerFactor = 0);

// Funções de comandos
void processSerialCommands();
void printHelp();

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
  // IMPORTANTE: Sempre processar se estiver em modo AP
  if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) {
    handleWiFiManager();
  }
  
  // Processar comandos Serial (não bloqueante)
  processSerialCommands();
  
  // AMOSTRAGEM CONTÍNUA: fazer amostragem instantânea de tensão e corrente
  sampleInstantaneousValues();
  
  // Calcular valores RMS e potência a cada intervalo configurado (1 segundo)
  if (lastCalculationTime == 0 || (currentTime - lastCalculationTime >= calculationIntervalMs)) {
    // Valores calculados
    float rmsVoltage = 0;
    float rmsCurrent = 0;
    float realPower = 0;
    float apparentPower = 0;
    float measuredPowerFactor = 0;
    
    // Calcular valores a partir dos acumuladores
    calculatePowerValues(rmsVoltage, rmsCurrent, realPower, apparentPower, measuredPowerFactor);
    
    // Verificar e ajustar offset (se necessário)
    checkAndAdjustOffset(rmsCurrent);
    
    // Calcular energia acumulada (usando potência real medida)
    // Usar valor absoluto da potência para energia (pode ser negativa se houver geração)
    if (samples > 0 && abs(realPower) > 0.001) {
      float durationHours = calculationIntervalMs / 3600000.0; // Converter ms para horas
      float energyWh = abs(realPower) * durationHours;
      totalEnergyWh += energyWh;
      saveCounter++;
    }
    
    // Mostrar valores no Serial Monitor
    Serial.print("Amostras: ");
    Serial.print(samples);
    Serial.print(", Vrms: ");
    Serial.print(rmsVoltage, 1);
    if (useRealVoltage) {
      Serial.print(" V (REAL)");
    } else {
      Serial.print(" V (FIXA)");
    }
    Serial.print(", Irms: ");
    Serial.print(rmsCurrent, 6);
    Serial.print(" A, P_ativa: ");
    Serial.print(realPower, 6);
    Serial.print(" W, S: ");
    Serial.print(apparentPower, 6);
    Serial.print(" VA, FP: ");
    Serial.print(measuredPowerFactor, 6);
    Serial.print(", Energia total: ");
    Serial.print(totalEnergyWh, 6);
    Serial.print(" Wh");
    
    // Calcular tempo restante para próximo salvamento
    unsigned long timeToNextSave = saveIntervalMs - (currentTime - lastSaveTime);
    if (timeToNextSave < saveIntervalMs) {
      Serial.print(", Próximo save em: ");
      Serial.print(timeToNextSave / 1000);
      Serial.print(" s");
    }
    Serial.println();
    
    // Resetar acumuladores para próximo intervalo
    sumV2 = 0.0;
    sumI2 = 0.0;
    sumP = 0.0;
    sumCurrentADC = 0.0;  // Resetar acumulador de ADC para recalibração
    samples = 0;
    lastCalculationTime = currentTime;
  }
  
  // Salvar na EEPROM a cada 10 cálculos (10 segundos)
  if (saveCounter >= 10) {
    saveEnergyToEEPROM();
    saveCounter = 0;
    Serial.println("💾 Energia salva na EEPROM!");
  }
  
  // Enviar dados para o servidor a cada 10 minutos
  if (WiFi.status() == WL_CONNECTED) {
    if (lastSaveTime == 0 || (currentTime - lastSaveTime >= saveIntervalMs)) {
      // Calcular valores finais para envio
      float rmsVoltage = 0;
      float rmsCurrent = 0;
      float realPower = 0;
      float apparentPower = 0;
      float measuredPowerFactor = 0;
      
      if (samples > 0) {
        calculatePowerValues(rmsVoltage, rmsCurrent, realPower, apparentPower, measuredPowerFactor);
      }
      
      sendDataToServer(totalEnergyWh, 10.0, realPower, apparentPower, measuredPowerFactor);
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
  
  // Pequeno delay para não sobrecarregar o processador
  delayMicroseconds(100);
}
