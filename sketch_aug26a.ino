const int sensorPin = 34;
const float VCC_ADC = 3.3;
const float sensitivity = 0.100; // ACS712 20A = 100 mV/A
const int adcResolution = 4095;
const float divisorFactor = 5.0 / 3.3; // sensor 5V, ADC 3.3V
const float lineVoltage = 220.0;       // tensão da rede em volts
const float powerFactor = 0.85;        // fator de potência típico para ventiladores

const int sampleDelayUs = 100;         // microsegundos entre amostras
const int samplesPerRMS = 500;         // amostras por cálculo RMS
const unsigned long printIntervalMs = 5000; // imprimir RMS/potência a cada 5s
const unsigned long totalMinutes = 10;       // total do experimento
const float noiseThreshold = 0.05;           // corrente mínima considerada (A)
const int movingAverageWindow = 10;          // janela média móvel

// variável global para offset calibrado
float voltageOffset = 0;

// buffer para média móvel
float rmsBuffer[movingAverageWindow];
int bufferIndex = 0;

// função para calibrar offset com carga desligada
void calibrateOffset(int numSamples = 1000) {
  float sum = 0;
  for (int i = 0; i < numSamples; i++) {
    int rawValue = analogRead(sensorPin);
    float voltage_adc = (rawValue * VCC_ADC) / adcResolution;
    float voltage_sensor = voltage_adc * divisorFactor;
    sum += voltage_sensor;
    delayMicroseconds(sampleDelayUs);
  }
  voltageOffset = sum / numSamples; // média das leituras com carga zero
  Serial.print("Offset calibrado: ");
  Serial.println(voltageOffset, 3);
}

// função para calcular média móvel do RMS
float movingAverage(float newValue) {
  rmsBuffer[bufferIndex] = newValue;
  bufferIndex = (bufferIndex + 1) % movingAverageWindow;
  float sum = 0;
  for (int i = 0; i < movingAverageWindow; i++) sum += rmsBuffer[i];
  return sum / movingAverageWindow;
}

void setup() {
  Serial.begin(115200);
  Serial.println("Calibrando offset, mantenha o ventilador desligado...");
  calibrateOffset();
  Serial.println("Calibração concluída. Iniciando medições...");

  // inicializa buffer RMS com 0
  for (int i = 0; i < movingAverageWindow; i++) rmsBuffer[i] = 0;
}

void loop() {
  float energyWh = 0.0;
  unsigned long experimentStart = millis();
  unsigned long lastPrint = 0;
  unsigned long lastLoopMillis = millis();

  while (millis() - experimentStart < totalMinutes * 60UL * 1000UL) {
    float sumSquares = 0;
    unsigned long loopStart = millis();

    // calcular RMS da corrente
    for (int i = 0; i < samplesPerRMS; i++) {
      int rawValue = analogRead(sensorPin);
      float voltage_adc = (rawValue * VCC_ADC) / adcResolution;
      float voltage_sensor = voltage_adc * divisorFactor;
      float currentInstant = (voltage_sensor - voltageOffset) / sensitivity;
      sumSquares += currentInstant * currentInstant;
      delayMicroseconds(sampleDelayUs);
    }

    float rmsCurrent = sqrt(sumSquares / samplesPerRMS);

    // aplicar threshold para ignorar ruído
    if (rmsCurrent < noiseThreshold) rmsCurrent = 0;

    // aplicar média móvel
    rmsCurrent = movingAverage(rmsCurrent);

    // potência real considerando fator de potência
    float powerWatts = rmsCurrent * lineVoltage * powerFactor;

    // calcular delta de tempo real em horas
    float deltaTimeHours = (millis() - lastLoopMillis) / 3600000.0;
    lastLoopMillis = millis();

    energyWh += powerWatts * deltaTimeHours;

    // imprimir corrente, potência e energia a cada 5s
    if (millis() - lastPrint >= printIntervalMs) {
      Serial.print("Corrente RMS: ");
      Serial.print(rmsCurrent, 2);
      Serial.print(" A, Potência: ");
      Serial.print(powerWatts, 2);
      Serial.print(" W, Energia acumulada: ");
      Serial.print(energyWh, 3);
      Serial.println(" Wh");
      lastPrint = millis();
    }
  }

  // resumo final após 10 minutos
  Serial.println("----- 10 minutos concluídos -----");
  Serial.print("Energia total acumulada: ");
  Serial.print(energyWh, 3);
  Serial.println(" Wh");

  while (true); // pausa final
}
