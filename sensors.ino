// ==========================================
// FUNÇÕES DE LEITURA DE SENSORES
// ==========================================

// Medir corrente e tensão RMS
void measureSensors(float& rmsCurrent, float& rmsVoltage) {
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
      float voltageAC = voltage_adc_voltage - voltageOffsetVoltage; // Remove offset DC
      sumVoltageSquares += voltageAC * voltageAC; // Acumula quadrados
      delayMicroseconds(1000); // Taxa de amostragem ~1kHz para tensão
    } else {
      delayMicroseconds(sampleDelayUs); // Delay padrão se tensão não estiver habilitada
    }
  }

  // Calcular RMS apenas se houver amostras válidas
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
}

// Verificar e ajustar offset automaticamente (se habilitado)
void checkAndAdjustOffset(float rmsCurrent) {
  int rawValue = analogRead(sensorPin);
  
  // VALIDAÇÃO CRÍTICA: Verificar se o sensor está funcionando
  if (rawValue == 0) {
    static unsigned long lastErrorTime = 0;
    if (millis() - lastErrorTime > 10000) { // Avisar a cada 10 segundos
      Serial.println("❌ ERRO CRÍTICO: ADC retornando 0! Sensor desconectado ou problema de hardware!");
      Serial.println("   Verifique a conexão do sensor ACS712 no pino 34");
      lastErrorTime = millis();
    }
    return; // Não processar se ADC = 0
  }
  
  float voltage_adc = (rawValue * VCC_ADC) / adcResolution;
  float voltage_sensor = voltage_adc * divisorFactor;
  
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
}

