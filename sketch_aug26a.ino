#include <WiFi.h>
#include <HTTPClient.h>
#include <EEPROM.h>

const char* ssid = "Tenda_618D50";
const char* password = "bleach309";
const char* serverUrl = "http://192.168.1.110:3000/readings"; // seu backend

const int sensorPin = 34;
const float VCC_ADC = 3.3;
const float sensitivity = 0.100; // ACS712 20A
const int adcResolution = 4095; // adcResolution é o valor máximo do ADC, ou seja, 12 bits = 2^12 = 4096
const float divisorFactor = 5.0 / 3.3;
const float lineVoltage = 220.0;
const float powerFactor = 0.85;
const int samplesPerRMS = 500;
const int sampleDelayUs = 100;
const float noiseThreshold = 0.1; // Aumentado para ignorar ruído
float voltageOffset = 0;
float totalEnergyWh = 0.0; // Energia acumulada total
int saveCounter = 0; // Contador para salvar periodicamente

// === Calibração ===
void calibrateOffset(int numSamples = 1000) {
  float sum = 0;
  for (int i = 0; i < numSamples; i++) {
    int rawValue = analogRead(sensorPin);
    float voltage_adc = (rawValue * VCC_ADC) / adcResolution;
    float voltage_sensor = voltage_adc * divisorFactor;
    sum += voltage_sensor;
    delayMicroseconds(sampleDelayUs);
  }
  voltageOffset = sum / numSamples;
  Serial.print("Offset calibrado: ");
  Serial.println(voltageOffset, 3);
}

// === Funções para EEPROM ===
void saveEnergyToEEPROM() {
  EEPROM.put(0, totalEnergyWh);
  EEPROM.commit();
}

void loadEnergyFromEEPROM() {
  EEPROM.get(0, totalEnergyWh);
  if (isnan(totalEnergyWh) || totalEnergyWh < 0) {
    totalEnergyWh = 0.0; // Reset se valor inválido
  }
  Serial.print("Energia carregada da EEPROM: ");
  Serial.print(totalEnergyWh, 6);
  Serial.println(" Wh");
}

void resetEnergy() {
  totalEnergyWh = 0.0;
  saveEnergyToEEPROM();
  Serial.println("Energia resetada!");
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // Inicializar EEPROM
  EEPROM.begin(512);
  
  // Carregar energia acumulada da EEPROM
  loadEnergyFromEEPROM();
  
  Serial.println("Conectando ao Wi-Fi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWi-Fi conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  calibrateOffset();
  
  Serial.println("\nComandos disponíveis:");
  Serial.println("'reset' - Resetar energia acumulada");
  Serial.println("'save' - Salvar energia atual na EEPROM");
  Serial.println("'status' - Mostrar status atual");
  Serial.println("'calibrate' - Recalibrar offset (desligue a carga primeiro)");
  Serial.println("'adjust' - Ajustar offset para valor atual");
  Serial.println("'debug' - Mostrar valores brutos do sensor");
}

void loop() {
  // Processar comandos Serial
  if (Serial.available()) {
    String command = Serial.readString();
    command.trim();
    
    if (command == "reset") {
      resetEnergy();
    } else if (command == "save") {
      saveEnergyToEEPROM();
      Serial.println("Energia salva na EEPROM!");
    } else if (command == "status") {
      Serial.print("Energia acumulada: ");
      Serial.print(totalEnergyWh, 6);
      Serial.println(" Wh");
    } else if (command == "calibrate") {
      Serial.println("Recalibrando offset...");
      calibrateOffset();
    } else if (command == "debug") {
      // Mostrar valores brutos sem processamento
      int rawValue = analogRead(sensorPin);
      float voltage_adc = (rawValue * VCC_ADC) / adcResolution;
      float voltage_sensor = voltage_adc * divisorFactor;
      Serial.print("Valores brutos - ADC: ");
      Serial.print(rawValue);
      Serial.print(", V_ADC: ");
      Serial.print(voltage_adc, 3);
      Serial.print(" V, V_Sensor: ");
      Serial.print(voltage_sensor, 3);
      Serial.print(" V, Offset: ");
      Serial.println(voltageOffset, 3);
    } else if (command == "adjust") {
      // Ajustar offset para valor atual
      int rawValue = analogRead(sensorPin);
      float voltage_adc = (rawValue * VCC_ADC) / adcResolution;
      float voltage_sensor = voltage_adc * divisorFactor;
      voltageOffset = voltage_sensor;
      Serial.print("Offset ajustado para: ");
      Serial.println(voltageOffset, 3);
    }
  }

  float sumSquares = 0;

  for (int i = 0; i < samplesPerRMS; i++) {
    int rawValue = analogRead(sensorPin);
    float voltage_adc = (rawValue * VCC_ADC) / adcResolution;
    float voltage_sensor = voltage_adc * divisorFactor;
    float currentInstant = (voltage_sensor - voltageOffset) / sensitivity;
    sumSquares += currentInstant * currentInstant;
    delayMicroseconds(sampleDelayUs);
  }

  float rmsCurrent = sqrt(sumSquares / samplesPerRMS);
  if (rmsCurrent < noiseThreshold) rmsCurrent = 0;

  float powerWatts = rmsCurrent * lineVoltage * powerFactor;
  float energyWh = powerWatts * (5.0 / 3600.0); // energia em Wh (5 segundos = 5/3600 horas)
  float durationMin = 5.0 / 60.0; // duração em minutos (5 segundos = 5/60 minutos)
  String port = "1"; // porta do dispositivo (você pode ajustar conforme necessário)

  // Acumular energia total
  totalEnergyWh += energyWh;
  saveCounter++;

  // Debug detalhado para investigar o problema
  int rawValue = analogRead(sensorPin);
  float voltage_adc = (rawValue * VCC_ADC) / adcResolution;
  float voltage_sensor = voltage_adc * divisorFactor;
  float currentInstant = (voltage_sensor - voltageOffset) / sensitivity;
  
  // Verificar se o offset mudou significativamente
  float offsetDiff = abs(voltage_sensor - voltageOffset);
  if (offsetDiff > 0.05) { // Se diferença > 50mV
    Serial.print("⚠️  OFFSET MUDOU! Diferença: ");
    Serial.print(offsetDiff, 3);
    Serial.print(" V (V_Sensor: ");
    Serial.print(voltage_sensor, 3);
    Serial.print(" V, Offset: ");
    Serial.print(voltageOffset, 3);
    Serial.println(" V)");
  }
  
  // Ajustar offset automaticamente se a diferença for pequena mas consistente
  if (offsetDiff > 0.01 && offsetDiff < 0.05) { // Entre 10mV e 50mV
    voltageOffset = (voltageOffset + voltage_sensor) / 2.0; // Média móvel do offset
    Serial.print("🔧 Ajustando offset para: ");
    Serial.println(voltageOffset, 3);
  }
  
  // Mostrar valores no Serial Monitor
  Serial.print("ADC: ");
  Serial.print(rawValue);
  Serial.print(", V_ADC: ");
  Serial.print(voltage_adc, 3);
  Serial.print(" V, V_Sensor: ");
  Serial.print(voltage_sensor, 3);
  Serial.print(" V, Offset: ");
  Serial.print(voltageOffset, 3);
  Serial.print(" V, Corrente instantânea: ");
  Serial.print(currentInstant, 4);
  Serial.print(" A, Corrente RMS: ");
  Serial.print(rmsCurrent, 3);
  Serial.print(" A, Potência: ");
  Serial.print(powerWatts, 2);
  Serial.print(" W, Energia (5s): ");
  Serial.print(energyWh, 6);
  Serial.print(" Wh, Total acumulado: ");
  Serial.print(totalEnergyWh, 6);
  Serial.println(" Wh");

  // Salvar na EEPROM a cada 10 leituras (50 segundos)
  if (saveCounter >= 10) {
    saveEnergyToEEPROM();
    saveCounter = 0;
    Serial.println("Energia salva automaticamente!");
  }

  // Enviar dados para o servidor
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");

    String jsonPayload = "{\"port\": \"" + port + "\"" +
                         ", \"energyWh\": " + String(totalEnergyWh, 6) + // enviar energia acumulada
                         ", \"durationMin\": " + String(durationMin) + "}"; // duração em minutos (5 segundos = 5/60 minutos)

    int httpResponseCode = http.POST(jsonPayload);

    if (httpResponseCode > 0) {
      Serial.print("Dados enviados: ");
      Serial.println(jsonPayload);
    } else {
      Serial.print("Erro HTTP: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  }

  delay(5000); // enviar a cada 5 segundos
}
