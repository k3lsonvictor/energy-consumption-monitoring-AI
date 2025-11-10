// ==========================================
// PROCESSAMENTO DE COMANDOS SERIAL
// ==========================================

#include <Preferences.h>

void processSerialCommands() {
  if (!Serial.available()) return;
  
  String command = Serial.readString();
  command.trim();
  
  if (command == "reset") {
    resetEnergy();
  } else if (command == "save") {
    // Forçar salvamento no banco
    sendDataToServer(totalEnergyWh, 10.0);
    lastSaveTime = millis();
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
  } else if (command == "time") {
    // Mostrar tempo restante para próximo salvamento
    unsigned long currentTime = millis();
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
  } else if (command == "wifi") {
    // Mostrar status do Wi-Fi
    Serial.println("=== STATUS Wi-Fi ===");
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Status: Conectado");
      Serial.print("IP: ");
      Serial.println(WiFi.localIP());
      Serial.print("RSSI: ");
      Serial.print(WiFi.RSSI());
      Serial.println(" dBm");
    } else if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) {
      Serial.println("Status: Modo Access Point (Configuração)");
      Serial.println("Rede: ESP32_Config");
      Serial.println("IP: 192.168.4.1");
      Serial.println("Acesse: http://192.168.4.1");
    } else {
      Serial.println("Status: Desconectado");
    }
    Serial.println("===================");
  } else if (command == "resetwifi") {
    // Resetar credenciais Wi-Fi
    Preferences prefs;
    prefs.begin("wifi-config", false);
    prefs.clear();
    prefs.end();
    Serial.println("✅ Credenciais Wi-Fi resetadas!");
    Serial.println("Reiniciando ESP32 em 2 segundos para entrar em modo configuração...");
    delay(2000);
    ESP.restart();
  } else if (command == "resetall") {
    // Resetar tudo: WiFi e energia
    Preferences prefs;
    prefs.begin("wifi-config", false);
    prefs.clear();
    prefs.end();
    resetEnergy();
    Serial.println("✅ Tudo resetado (WiFi + Energia)!");
    Serial.println("Reiniciando ESP32 em 2 segundos...");
    delay(2000);
    ESP.restart();
  }
}

void printHelp() {
  Serial.println("\nComandos disponíveis:");
  Serial.println("'reset' - Resetar energia acumulada");
  Serial.println("'save' - Forçar salvamento no banco de dados");
  Serial.println("'status' - Mostrar status atual");
  Serial.println("'time' - Mostrar tempo para próximo salvamento");
  Serial.println("'wifi' - Mostrar status do Wi-Fi");
  Serial.println("'resetwifi' - Resetar credenciais Wi-Fi (reinicia automaticamente)");
  Serial.println("'resetall' - Resetar tudo (WiFi + Energia)");
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

