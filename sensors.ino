// ==========================================
// FUNÇÕES DE LEITURA DE SENSORES
// ==========================================

// Amostragem instantânea simultânea de tensão e corrente
// Esta função é chamada continuamente no loop para acumular valores
void sampleInstantaneousValues() {
  // AMOSTRAGEM SIMULTÂNEA: ler tensão e corrente o mais rápido possível
  int rawCurrent = analogRead(sensorPin);      // Leitura de corrente (ACS712)
  int rawVoltage = useRealVoltage ? analogRead(voltagePin) : 0; // Leitura de tensão (ZMPT101B)
  
  // Processar corrente
  float currentInstant = 0.0;
  bool currentValid = false;
  
  if (rawCurrent > 0) { // Validar se o sensor está funcionando
    // 1. Converter ADC para Volts na saída do módulo
    float voltage_adc_current = (rawCurrent * VCC_ADC) / adcResolution;
    float voltage_sensor_current = voltage_adc_current * divisorFactor;
    
    // 2. Remover offset (ponto médio)
    // 3. Converter para corrente em Ampères usando constante de calibração
    // Validar se a tensão está em faixa válida (0.1V a 5.0V)
    // A corrente pode ser positiva ou negativa após remover o offset
    if (voltage_sensor_current > 0.1 && voltage_sensor_current < 5.0) {
      currentInstant = (voltage_sensor_current - voltageOffsetCurrent) / sensitivity;
      // Aceitar corrente mesmo se for muito pequena (pode ser negativa ou positiva)
      // A validação é apenas se o sensor está funcionando (voltage_sensor_current em faixa)
      currentValid = true;
    }
  }
  
  // Processar tensão
  float voltageInstant = 0.0;
  bool voltageValid = false;
  
  if (useRealVoltage) {
    if (rawVoltage > 0) {
      // 1. Converter ADC para Volts na saída do módulo
      float voltage_adc_voltage = (rawVoltage * VCC_ADC) / adcResolution;
      
      // 2. Remover offset (ponto médio)
      float voltageAC_sensor = voltage_adc_voltage - voltageOffsetVoltage;
      
      // 3. Converter para tensão da rede em Volts usando constante de calibração
      voltageInstant = voltageAC_sensor / voltageSensitivity;
      
      // Validar se a tensão está em faixa razoável (0-400V para rede elétrica)
      if (abs(voltageInstant) < 400.0) {
        voltageValid = true;
      }
    }
  } else {
    // Se não usar tensão real, gerar onda senoidal sintética
    // Assumindo tensão senoidal perfeita: v(t) = Vrms * √2 * sin(2πft)
    static unsigned long startTime = 0;
    if (startTime == 0) {
      startTime = micros();
    }
    const float frequency = 60.0;       // Frequência da rede (Hz)
    const float twoPiF = 2.0 * PI * frequency;
    unsigned long currentTime = micros();
    float timeSeconds = (currentTime - startTime) / 1000000.0;
    float voltagePeak = lineVoltage * sqrt(2.0); // Tensão de pico (Vrms * √2)
    voltageInstant = voltagePeak * sin(twoPiF * timeSeconds);
    voltageValid = true;
  }
  
  // Se ambas as leituras são válidas, acumular valores
  if (currentValid && voltageValid) {
    // Acumular quadrados para cálculo RMS
    sumV2 += voltageInstant * voltageInstant;  // v²
    sumI2 += currentInstant * currentInstant;  // i²
    
    // Acumular potência instantânea: p(t) = v(t) * i(t)
    float instantPower = voltageInstant * currentInstant;
    sumP += instantPower;
    
    samples++;
    
    // Debug: mostrar algumas amostras para diagnóstico
    static unsigned long lastSampleDebug = 0;
    static int sampleDebugCount = 0;
    if (millis() - lastSampleDebug > 2000 && sampleDebugCount < 5) {
      Serial.print("DEBUG Amostra #");
      Serial.print(sampleDebugCount);
      Serial.print(": v=");
      Serial.print(voltageInstant, 3);
      Serial.print("V, i=");
      Serial.print(currentInstant, 6);
      Serial.print("A, p=");
      Serial.print(instantPower, 6);
      Serial.print("W, samples=");
      Serial.println(samples);
      lastSampleDebug = millis();
      sampleDebugCount++;
    }
  } else {
    // Debug: mostrar por que amostras estão sendo rejeitadas
    static unsigned long lastRejectDebug = 0;
    static int rejectCount = 0;
    if (millis() - lastRejectDebug > 5000 && rejectCount < 3) {
      Serial.print("DEBUG: Amostra rejeitada - currentValid=");
      Serial.print(currentValid);
      Serial.print(", voltageValid=");
      Serial.println(voltageValid);
      if (!currentValid) {
        Serial.println("  Razão: Corrente inválida (rawCurrent=0 ou fora de faixa)");
      }
      if (!voltageValid) {
        Serial.println("  Razão: Tensão inválida (rawVoltage=0 ou fora de faixa)");
      }
      lastRejectDebug = millis();
      rejectCount++;
    }
  }
  
  // Pequeno delay para estabilizar ADC (importante para leituras precisas)
  delayMicroseconds(100);
}

// Calcular valores RMS e potência a partir dos acumuladores
void calculatePowerValues(float& rmsVoltage, float& rmsCurrent, float& realPower, float& apparentPower, float& powerFactor) {
  if (samples > 0) {
    // Calcular Vrms: sqrt(1/N * sum(v_k^2))
    if (useRealVoltage) {
      rmsVoltage = sqrt(sumV2 / samples);
    } else {
      // Para onda senoidal sintética, calcular RMS a partir das amostras
      rmsVoltage = sqrt(sumV2 / samples);
      // Se o cálculo der muito diferente de lineVoltage, usar lineVoltage como fallback
      if (abs(rmsVoltage - lineVoltage) > lineVoltage * 0.1) {
        rmsVoltage = lineVoltage;
      }
    }
    
    // Calcular Irms: sqrt(1/N * sum(i_k^2))
    rmsCurrent = sqrt(sumI2 / samples);
    // NÃO zerar corrente aqui - deixar o valor real para cálculo de potência
    // Apenas zerar para exibição se necessário
    
    // Calcular potência ativa: P = (1/N) * sum(p_k) = (1/N) * sum(v_k * i_k)
    realPower = sumP / samples;
    
    // Se a potência for negativa, pode ser:
    // 1. Fase invertida (problema de conexão ou calibração)
    // 2. Geração de energia (inversor, etc)
    // 3. Offset incorreto causando deslocamento de fase
    // Usar valor absoluto para potência e FP (assumindo que é erro de calibração/fase)
    bool powerIsNegative = (realPower < 0);
    if (powerIsNegative) {
      static unsigned long lastNegativePowerWarning = 0;
      if (millis() - lastNegativePowerWarning > 10000) {
        Serial.println("⚠️  AVISO: Potência negativa detectada!");
        Serial.println("   Possíveis causas:");
        Serial.println("   1. Offset incorreto (execute 'calibrate')");
        Serial.println("   2. Fase invertida (verifique conexões)");
        Serial.println("   3. Geração de energia (inversor)");
        Serial.print("   Usando valor absoluto: ");
        Serial.print(abs(realPower), 2);
        Serial.println(" W");
        lastNegativePowerWarning = millis();
      }
      // Usar valor absoluto para potência
      realPower = abs(realPower);
    }
    
    // Calcular potência aparente: S = Vrms * Irms
    apparentPower = rmsVoltage * rmsCurrent;
    
    // Calcular fator de potência: FP = P / S
    // Usar a potência corrigida (já com abs se necessário) para calcular FP
    if (apparentPower > 0.001) { // Evitar divisão por zero
      powerFactor = realPower / apparentPower;
      // Limitar FP entre 0 e 1 (sempre positivo, pois usamos abs da potência)
      if (powerFactor > 1.0) powerFactor = 1.0;
      if (powerFactor < 0.0) powerFactor = 0.0;
    } else {
      powerFactor = 0.0;
    }
    
    // Debug: mostrar valores intermediários periodicamente
    static unsigned long lastDebugTime = 0;
    static int debugCount = 0;
    if (millis() - lastDebugTime > 5000 || debugCount < 3) { // A cada 5 segundos ou nas primeiras 3 vezes
      Serial.println("=== DEBUG CÁLCULO ===");
      Serial.print("Samples coletadas: ");
      Serial.println(samples);
      Serial.print("sumV2 acumulado: ");
      Serial.println(sumV2, 6);
      Serial.print("sumI2 acumulado: ");
      Serial.println(sumI2, 6);
      Serial.print("sumP acumulado: ");
      Serial.println(sumP, 6);
      Serial.print("Vrms calculado: ");
      Serial.println(rmsVoltage, 3);
      Serial.print("Irms calculado: ");
      Serial.println(rmsCurrent, 6);
      Serial.print("P_ativa calculada (sumP/samples): ");
      Serial.println(realPower, 6);
      Serial.print("S calculada (Vrms*Irms): ");
      Serial.println(apparentPower, 6);
      Serial.print("FP calculado (P/S): ");
      Serial.println(powerFactor, 6);
      Serial.print("useRealVoltage: ");
      Serial.println(useRealVoltage ? "SIM" : "NÃO");
      Serial.print("Potência original (antes do abs): ");
      Serial.print(sumP / samples, 6);
      Serial.println(" W");
      if (sumP / samples < 0) {
        Serial.println("⚠️  Potência negativa - verifique offset e calibração!");
      }
      Serial.println("====================");
      lastDebugTime = millis();
      debugCount++;
    }
  } else {
    // Se não houver amostras válidas, zerar todos os valores
    rmsCurrent = 0;
    rmsVoltage = useRealVoltage ? 0 : lineVoltage;
    realPower = 0;
    apparentPower = 0;
    powerFactor = 0;
    
    static unsigned long lastWarning = 0;
    if (millis() - lastWarning > 5000) {
      Serial.println("⚠️  AVISO: Nenhuma amostra válida coletada! Verifique sensores.");
      lastWarning = millis();
    }
  }
}

// Medir corrente e tensão RMS (função legada - mantida para compatibilidade)
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

