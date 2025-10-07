#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "NOME_DA_REDE";
const char* password = "SENHA_DA_REDE";
const char* serverUrl = "http://192.168.0.10:8000/api/measurements"; // seu backend

const int sensorPin = 34;
const float VCC_ADC = 3.3;
const float sensitivity = 0.100; // ACS712 20A
const int adcResolution = 4095;
const float divisorFactor = 5.0 / 3.3;
const float lineVoltage = 220.0;
const float powerFactor = 0.85;
const int samplesPerRMS = 500;
const int sampleDelayUs = 100;
const float noiseThreshold = 0.05;
float voltageOffset = 0;

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

void setup() {
  Serial.begin(115200);
  delay(1000);
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
}

void loop() {
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

  // Enviar dados para o servidor
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");

    String jsonPayload = "{\"current\": " + String(rmsCurrent, 3) +
                         ", \"power\": " + String(powerWatts, 3) +
                         ", \"timestamp\": " + String(millis()) + "}";

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
