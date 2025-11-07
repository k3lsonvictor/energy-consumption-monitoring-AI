// ==========================================
// CONFIGURAÇÕES E CONSTANTES
// ==========================================

// Configurações Wi-Fi
// Nota: SSID e password são salvos via Preferences (WiFi Manager)
const char* serverUrl = "http://192.168.1.110:3000/readings";

// Pinos dos sensores
const int sensorPin = 34;        // Sensor de corrente (ACS712)
const int voltagePin = 35;       // Sensor de tensão (ZMPT101B)

// Configurações ADC
const float VCC_ADC = 3.3;       // Tensão de referência ADC
const int adcResolution = 4095;  // Resolução 12 bits = 2^12 - 1

// Configurações do sensor de corrente (ACS712)
const float sensitivity = 0.100; // ACS712 20A = 100mV/A
const float divisorFactor = 5.0 / 3.3; // Conversão ADC (3.3V) para sensor (5V)

// Configurações do sensor de tensão (ZMPT101B)
float voltageSensitivity = 0.0017; // Sensibilidade (ajuste até bater com 220V RMS)

// Configurações de medição
const float lineVoltage = 220.0; // Tensão nominal (fallback)
const float powerFactor = 0.85;
const int samplesPerRMS = 500;
const int sampleDelayUs = 100;
const float noiseThreshold = 0.2; // Threshold para ignorar ruído

// Configurações de salvamento
const unsigned long saveIntervalMs = 600000; // 10 minutos em millisegundos

