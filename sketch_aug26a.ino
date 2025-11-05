#include <WiFi.h>
#include <HTTPClient.h>
#include <EEPROM.h>

const char* ssid = "Tenda_618D50";
const char* password = "bleach309";
const char* serverUrl = "http://192.168.1.110:3000/readings"; // seu backend

const int sensorPin = 34;        // Sensor de corrente
const int voltagePin = 35;       // Sensor de tensão (ADC)
const float VCC_ADC = 3.3;       // Tensão de referência ADC
const float sensitivity = 0.100; // ACS712 20A
const int adcResolution = 4095; // adcResolution é o valor máximo do ADC, ou seja, 12 bits = 2^12 = 4096
const float divisorFactor = 5.0 / 3.3;
float voltageSensitivity = 0.0017; // Sensibilidade do ZMPT101B (ajuste até bater com 220V RMS)
const float lineVoltage = 220.0; // Tensão nominal (fallback)
const float powerFactor = 0.85;
const int samplesPerRMS = 500;
const int sampleDelayUs = 100;
const float noiseThreshold = 0.2; // Aumentado para ignorar ruído e flutuações
const unsigned long saveIntervalMs = 600000; // 10 minutos em millisegundos
float voltageOffsetCurrent = 0;
float voltageOffsetVoltage = 2.5; // Offset para medição de tensão (ZMPT101B ~Vcc/2)
float totalEnergyWh = 0.0; // Energia acumulada total
int saveCounter = 0; // Contador para salvar periodicamente
unsigned long lastSaveTime = 0; // Última vez que salvou no banco
bool autoOffsetAdjust = false; // Desabilitar ajuste automático por padrão
bool useRealVoltage = true; // Usar tensão real ou fixa

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
  voltageOffsetCurrent = sum / numSamples;
  Serial.print("Offset de corrente calibrado: ");
  Serial.println(voltageOffsetCurrent, 3);
}

void calibrateVoltageOffset(int numSamples = 1000) {
  float sum = 0;
  Serial.println("Calibrando offset de tensão (ZMPT101B)...");
  Serial.println("Aguarde, medindo offset DC...");
  for (int i = 0; i < numSamples; i++) {
    int rawValue = analogRead(voltagePin);
    float voltage_adc = rawValue * (VCC_ADC / adcResolution); // Converte para volts (0-3.3V)
    sum += voltage_adc;
    delayMicroseconds(1000); // Taxa de amostragem ~1kHz
    if (i % 200 == 0) {
      Serial.print(".");
    }
  }
  voltageOffsetVoltage = sum / numSamples;
  Serial.println();
  Serial.print("Offset de tensão calibrado: ");
  Serial.print(voltageOffsetVoltage, 3);
  Serial.println(" V (deve estar próximo de 2.5V para ZMPT101B)");
  Serial.print("Sensibilidade atual: ");
  Serial.println(voltageSensitivity, 4);
  Serial.println("Use 'setvsens:VALOR' para ajustar sensibilidade se necessário");
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
  
  // Configurar ADC para medição de tensão
  analogReadResolution(12); // Resolução 12 bits
  analogSetPinAttenuation(voltagePin, ADC_11db); // Até ~3.6V (para ZMPT101B)
  
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
  
  // Aviso sobre medição de tensão
  if (!useRealVoltage) {
    Serial.println("\n⚠️  ATENÇÃO: Medição de tensão real DESABILITADA");
    Serial.println("   Sistema usando tensão fixa de 220V");
    Serial.println("   Execute 'voltage' para habilitar medição real (ZMPT101B)");
  } else {
    Serial.println("\n✅ Medição de tensão real HABILITADA (ZMPT101B)");
  }
  
  Serial.println("\nComandos disponíveis:");
  Serial.println("'reset' - Resetar energia acumulada");
  Serial.println("'save' - Forçar salvamento no banco de dados");
  Serial.println("'status' - Mostrar status atual");
  Serial.println("'time' - Mostrar tempo para próximo salvamento");
  Serial.println("'fix' - Calibrar e travar offset (RECOMENDADO)");
  Serial.println("'calibrate' - Recalibrar offset de corrente");
  Serial.println("'adjust' - Ajustar offset para valor atual");
  Serial.println("'voltage' - Toggle medição de tensão real (ZMPT101B)");
  Serial.println("'calvoltage' - Calibrar offset de tensão");
  Serial.println("'vsens' - Mostrar sensibilidade de tensão");
  Serial.println("'setvsens:VALOR' - Ajustar sensibilidade (ex: setvsens:0.0017)");
  Serial.println("'auto' - Toggle ajuste automático do offset");
  Serial.println("'lock' - Travar offset atual");
  Serial.println("'debug' - Mostrar valores brutos do sensor");
}

void loop() {
  unsigned long currentTime = millis(); // Declarar uma vez no início
  
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
      Serial.println("=== STATUS DO SISTEMA ===");
      Serial.print("Energia acumulada: ");
      Serial.print(totalEnergyWh, 6);
      Serial.println(" Wh");
      Serial.print("Medição de tensão: ");
      Serial.println(useRealVoltage ? "REAL (ZMPT101B)" : "FIXA (220V)");
      if (useRealVoltage) {
        Serial.print("Sensibilidade: ");
        Serial.println(voltageSensitivity, 4);
        Serial.print("Offset tensão: ");
        Serial.print(voltageOffsetVoltage, 3);
        Serial.println(" V");
      }
      Serial.println("========================");
    } else if (command == "calibrate") {
      Serial.println("Recalibrando offset...");
      calibrateOffset();
      autoOffsetAdjust = false; // Trava o offset após calibração
      Serial.println("Offset calibrado e travado!");
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
      Serial.println(voltageOffsetCurrent, 3);
    } else if (command == "adjust") {
      // Ajustar offset para valor atual
      int rawValue = analogRead(sensorPin);
      float voltage_adc = (rawValue * VCC_ADC) / adcResolution;
      float voltage_sensor = voltage_adc * divisorFactor;
      voltageOffsetCurrent = voltage_sensor;
      Serial.print("Offset ajustado para: ");
      Serial.println(voltageOffsetCurrent, 3);
    } else if (command == "save") {
      // Forçar salvamento no banco
      if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(serverUrl);
        http.addHeader("Content-Type", "application/json");

        String port = "1";
        float durationMin = 10.0;
        
        String jsonPayload = "{\"port\": \"" + port + "\"" +
                             ", \"energyWh\": " + String(totalEnergyWh, 6) +
                             ", \"durationMin\": " + String(durationMin) + "}";

        int httpResponseCode = http.POST(jsonPayload);

        if (httpResponseCode > 0) {
          Serial.println("=== SALVAMENTO FORÇADO ===");
          Serial.print("Energia: ");
          Serial.print(totalEnergyWh, 6);
          Serial.println(" Wh");
          Serial.println("Dados salvos no banco!");
          Serial.println("========================");
          lastSaveTime = millis();
        } else {
          Serial.print("Erro HTTP: ");
          Serial.println(httpResponseCode);
        }

        http.end();
      } else {
        Serial.println("WiFi desconectado!");
      }
    } else if (command == "time") {
      // Mostrar tempo restante para próximo salvamento
      unsigned long timeSinceLastSave = currentTime - lastSaveTime;
      unsigned long timeToNextSave = saveIntervalMs - timeSinceLastSave;
      
      Serial.print("Tempo desde último salvamento: ");
      Serial.print(timeSinceLastSave / 1000);
      Serial.println(" segundos");
      Serial.print("Tempo para próximo salvamento: ");
      Serial.print(timeToNextSave / 1000);
      Serial.println(" segundos");
    } else if (command == "auto") {
      // Toggle ajuste automático do offset
      autoOffsetAdjust = !autoOffsetAdjust;
      Serial.print("Ajuste automático do offset: ");
      Serial.println(autoOffsetAdjust ? "HABILITADO" : "DESABILITADO");
    } else if (command == "lock") {
      // Travar offset atual
      autoOffsetAdjust = false;
      Serial.println("Offset travado! Use 'adjust' ou 'calibrate' para alterar.");
    } else if (command == "fix") {
      // Calibrar e travar offset
      Serial.println("Calibrando e travando offset...");
      calibrateOffset();
      autoOffsetAdjust = false;
      Serial.println("Offset calibrado e travado! Sistema estabilizado.");
    } else if (command == "voltage") {
      // Toggle medição de tensão real
      useRealVoltage = !useRealVoltage;
      if (useRealVoltage) {
        Serial.println("=== Medição de tensão real HABILITADA ===");
        Serial.println("Sensor: ZMPT101B");
        Serial.print("Sensibilidade atual: ");
        Serial.println(voltageSensitivity, 4);
        Serial.print("Offset atual: ");
        Serial.print(voltageOffsetVoltage, 3);
        Serial.println(" V");
        Serial.println("1. Execute 'calvoltage' para calibrar offset");
        Serial.println("2. Use 'setvsens:VALOR' para ajustar sensibilidade (ex: setvsens:0.0017)");
        Serial.println("3. Compare com multímetro para validar");
        Serial.println("==========================================");
      } else {
        Serial.println("Medição de tensão real DESABILITADA (usando 220V fixo)");
      }
    } else if (command == "calvoltage") {
      // Calibrar tensão
      Serial.println("Calibrando tensão...");
      calibrateVoltageOffset();
      Serial.println("Tensão calibrada!");
    } else if (command.startsWith("setvsens:")) {
      // Ajustar sensibilidade de tensão: setvsens:0.0017
      float newSensitivity = command.substring(9).toFloat();
      if (newSensitivity > 0 && newSensitivity < 0.01) {
        voltageSensitivity = newSensitivity;
        Serial.print("Sensibilidade de tensão ajustada para: ");
        Serial.println(voltageSensitivity, 4);
        Serial.println("Compare com multímetro para validar!");
      } else {
        Serial.println("Valor inválido! Use: setvsens:0.0017 (valor típico entre 0.0015-0.0020)");
      }
    } else if (command == "vsens") {
      // Mostrar sensibilidade atual
      Serial.print("Sensibilidade de tensão atual: ");
      Serial.println(voltageSensitivity, 4);
      Serial.println("Use 'setvsens:VALOR' para ajustar (ex: setvsens:0.0017)");
      Serial.println("Valor típico: entre 0.0015 e 0.0020");
    }
  }

  float sumSquares = 0;
  float sumVoltageSquares = 0;
  int validSamples = 0; // Contador de amostras válidas

  for (int i = 0; i < samplesPerRMS; i++) {
    // Medir corrente
    int rawValue = analogRead(sensorPin);
    
    // Validar se o sensor está funcionando
    if (rawValue > 0) { // Apenas calcular se ADC retornar valor válido
      float voltage_adc = (rawValue * VCC_ADC) / adcResolution;
      float voltage_sensor = voltage_adc * divisorFactor;
      
      // Validar se a tensão está em faixa válida
      if (voltage_sensor > 0.1 && voltage_sensor < 5.0) {
        float currentInstant = (voltage_sensor - voltageOffsetCurrent) / sensitivity;
        sumSquares += currentInstant * currentInstant;
        validSamples++;
      }
    }
    
    // Medir tensão (se habilitado)
    if (useRealVoltage) {
      int voltageRawValue = analogRead(voltagePin);
      float voltage_adc_voltage = voltageRawValue * (VCC_ADC / adcResolution); // Converte para volts (0-3.3V)
      float voltageAC = voltage_adc_voltage - voltageOffsetVoltageCurrent; // Remove offset DC
      sumVoltageSquares += voltageAC * voltageAC; // Acumula quadrados
      delayMicroseconds(1000); // Taxa de amostragem ~1kHz para tensão
    } else {
      delayMicroseconds(sampleDelayUs); // Delay padrão se tensão não estiver habilitada
    }
  }

  // Calcular RMS apenas se houver amostras válidas
  float rmsCurrent = 0;
  if (validSamples > 0) {
    rmsCurrent = sqrt(sumSquares / validSamples);
    if (rmsCurrent < noiseThreshold) rmsCurrent = 0;
  } else {
    // Se não houver amostras válidas, não calcular corrente
    rmsCurrent = 0;
    static unsigned long lastNoSamplesWarning = 0;
    if (millis() - lastNoSamplesWarning > 10000) {
      Serial.println("⚠️  Aviso: Nenhuma amostra válida de corrente detectada!");
      lastNoSamplesWarning = millis();
    }
  }

  // Calcular tensão RMS real ou usar fixa
  float rmsVoltage;
  if (useRealVoltage) {
    if (sumVoltageSquares > 0) {
      float valorRMSsensor = sqrt(sumVoltageSquares / samplesPerRMS); // Valor RMS em volts (sensor)
      rmsVoltage = valorRMSsensor / voltageSensitivity; // Converte para Volts reais (rede)
    } else {
      // Se não houver leituras, usar fixa temporariamente
      rmsVoltage = lineVoltage;
      Serial.println("⚠️  Aviso: Nenhuma leitura de tensão detectada, usando valor fixo");
    }
  } else {
    rmsVoltage = lineVoltage; // Usar tensão fixa
  }

  float powerWatts = rmsCurrent * rmsVoltage * powerFactor;
  float energyWh = powerWatts * (5.0 / 3600.0); // energia em Wh (5 segundos = 5/3600 horas)
  float durationMin = 5.0 / 60.0; // duração em minutos (5 segundos = 5/60 minutos)
  String port = "1"; // porta do dispositivo (você pode ajustar conforme necessário)

  // Acumular energia total APENAS se houver leituras válidas
  if (validSamples > 0) {
    totalEnergyWh += energyWh;
    saveCounter++;
  }

  // Debug detalhado para investigar o problema
  int rawValue = analogRead(sensorPin);
  
  // VALIDAÇÃO CRÍTICA: Verificar se o sensor está funcionando
  if (rawValue == 0) {
    static unsigned long lastErrorTime = 0;
    if (millis() - lastErrorTime > 10000) { // Avisar a cada 10 segundos
      Serial.println("❌ ERRO CRÍTICO: ADC retornando 0! Sensor desconectado ou problema de hardware!");
      Serial.println("   Verifique a conexão do sensor ACS712 no pino 34");
      lastErrorTime = millis();
    }
    // Não calcular nada com valores inválidos
    delay(5000);
    return; // Pular esta iteração
  }
  
  float voltage_adc = (rawValue * VCC_ADC) / adcResolution;
  float voltage_sensor = voltage_adc * divisorFactor;
  float currentInstant = (voltage_sensor - voltageOffsetCurrent) / sensitivity;
  
  // Verificar se o offset mudou significativamente (APENAS se valores são válidos)
  if (voltage_sensor > 0.1 && voltage_sensor < 5.0) { // Valores válidos entre 0.1V e 5V
    float offsetDiff = abs(voltage_sensor - voltageOffsetCurrent);
    
    // Ajustar offset automaticamente apenas se habilitado e condições muito restritivas
    if (autoOffsetAdjust && offsetDiff > 0.005 && offsetDiff < 0.02 && rmsCurrent < 0.02) {
      voltageOffsetCurrent = (voltageOffsetCurrent * 0.95) + (voltage_sensor * 0.05); // Ajuste muito suave (95% antigo + 5% novo)
      Serial.print("🔧 Ajustando offset muito suavemente para: ");
      Serial.println(voltageOffsetCurrent, 3);
    } else if (offsetDiff > 0.05) { // Se diferença > 50mV (mas não ajustar automaticamente)
      static unsigned long lastWarningTime = 0;
      if (millis() - lastWarningTime > 5000) { // Avisar a cada 5 segundos
        Serial.print("⚠️  OFFSET MUDOU! Diferença: ");
        Serial.print(offsetDiff, 3);
        Serial.print(" V (V_Sensor: ");
        Serial.print(voltage_sensor, 3);
        Serial.print(" V, Offset: ");
        Serial.print(voltageOffsetCurrent, 3);
        Serial.println(" V)");
        Serial.println("   Use 'adjust' ou 'calibrate' para corrigir");
        lastWarningTime = millis();
      }
    }
  }
  
  // Calcular tempo restante para próximo salvamento
  unsigned long timeToNextSave = saveIntervalMs - (currentTime - lastSaveTime);
  
  // Mostrar valores no Serial Monitor
  Serial.print("ADC: ");
  Serial.print(rawValue);
  if (rawValue == 0) {
    Serial.print(" (ERRO!)");
  }
  Serial.print(", Amostras válidas: ");
  Serial.print(validSamples);
  Serial.print("/");
  Serial.print(samplesPerRMS);
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
  if (WiFi.status() == WL_CONNECTED && (currentTime - lastSaveTime >= saveIntervalMs)) {
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");

    String port = "1";
    float durationMin = 10.0; // 10 minutos
    
    String jsonPayload = "{\"port\": \"" + port + "\"" +
                         ", \"energyWh\": " + String(totalEnergyWh, 6) + // enviar energia acumulada
                         ", \"durationMin\": " + String(durationMin) + "}";

    int httpResponseCode = http.POST(jsonPayload);

    if (httpResponseCode > 0) {
      Serial.println("=== DADOS SALVOS NO BANCO ===");
      Serial.print("Energia acumulada: ");
      Serial.print(totalEnergyWh, 6);
      Serial.println(" Wh");
      Serial.print("Dados enviados: ");
      Serial.println(jsonPayload);
      Serial.println("=============================");
      lastSaveTime = currentTime;
    } else {
      Serial.print("Erro HTTP ao salvar: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  }

  delay(5000); // medir a cada 5 segundos
}
