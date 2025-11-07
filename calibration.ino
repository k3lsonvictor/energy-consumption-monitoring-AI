// ==========================================
// FUNÇÕES DE CALIBRAÇÃO
// ==========================================

// Calibrar offset do sensor de corrente (ACS712)
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

// Calibrar offset do sensor de tensão (ZMPT101B)
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

