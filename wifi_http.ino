// ==========================================
// FUNÇÕES HTTP
// ==========================================

// Enviar dados para o servidor
void sendDataToServer(float energy, float duration, float realPower, float apparentPower, float powerFactor) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠️  WiFi desconectado! Não é possível salvar no banco.");
    return;
  }

  HTTPClient http;
  http.setTimeout(10000); // Timeout de 10 segundos
  http.begin(serverUrl);
  http.addHeader("Content-Type", "application/json");

  String port = "1";
  
  // Montar JSON com todos os dados de potência
  String jsonPayload = "{\"port\": \"" + port + "\"" +
                       ", \"energyWh\": " + String(energy, 6) +
                       ", \"durationMin\": " + String(duration) +
                       ", \"realPower\": " + String(realPower, 2) +
                       ", \"apparentPower\": " + String(apparentPower, 2) +
                       ", \"powerFactor\": " + String(powerFactor, 3) + "}";

  Serial.print("Enviando: ");
  Serial.println(jsonPayload);
  
  int httpResponseCode = http.POST(jsonPayload);

  if (httpResponseCode > 0) {
    Serial.println("=== DADOS SALVOS NO BANCO ===");
    Serial.print("HTTP Response Code: ");
    Serial.println(httpResponseCode);
    Serial.print("Energia acumulada: ");
    Serial.print(energy, 6);
    Serial.println(" Wh");
    Serial.print("Potência ativa: ");
    Serial.print(realPower, 2);
    Serial.println(" W");
    Serial.print("Potência aparente: ");
    Serial.print(apparentPower, 2);
    Serial.println(" VA");
    Serial.print("Fator de potência: ");
    Serial.println(powerFactor, 3);
    Serial.print("Dados enviados: ");
    Serial.println(jsonPayload);
    
    // Ler resposta do servidor
    String response = http.getString();
    if (response.length() > 0) {
      Serial.print("Resposta do servidor: ");
      Serial.println(response);
    }
    
    Serial.println("=============================");
  } else {
    Serial.println("❌ ERRO ao salvar no banco!");
    Serial.print("HTTP Response Code: ");
    Serial.println(httpResponseCode);
    
    // Ler resposta de erro se disponível
    String response = http.getString();
    if (response.length() > 0) {
      Serial.print("Resposta do servidor: ");
      Serial.println(response);
    }
    
    Serial.println("Tente novamente com o comando 'save'");
  }

  http.end();
}

// Enviar apenas potência ativa (realPower) para o servidor (a cada 5 minutos)
// realPower = potência ativa em Watts (P = média de v(t) * i(t))
void sendPowerToServer(float realPower) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠️  WiFi desconectado! Não é possível salvar potência no banco.");
    return;
  }

  HTTPClient http;
  http.setTimeout(10000); // Timeout de 10 segundos
  http.begin(powerServerUrl); // Usar endpoint específico para potência
  http.addHeader("Content-Type", "application/json");

  String port = "1";
  
  // Montar JSON apenas com potência ativa (realPower) em Watts
  // realPower é calculado como: P = (1/N) * sum(v_k * i_k)
  String jsonPayload = "{\"port\": \"" + port + "\"" +
                       ", \"realPower\": " + String(realPower, 2) + "}";

  Serial.print("Enviando potência: ");
  Serial.println(jsonPayload);
  
  int httpResponseCode = http.POST(jsonPayload);

  if (httpResponseCode > 0) {
    Serial.println("=== POTÊNCIA SALVA NO BANCO ===");
    Serial.print("HTTP Response Code: ");
    Serial.println(httpResponseCode);
    Serial.print("Potência atual: ");
    Serial.print(realPower, 2);
    Serial.println(" W");
    
    // Ler resposta do servidor
    String response = http.getString();
    if (response.length() > 0) {
      Serial.print("Resposta do servidor: ");
      Serial.println(response);
    }
    
    Serial.println("=============================");
  } else {
    Serial.println("❌ ERRO ao salvar potência no banco!");
    Serial.print("HTTP Response Code: ");
    Serial.println(httpResponseCode);
    
    // Ler resposta de erro se disponível
    String response = http.getString();
    if (response.length() > 0) {
      Serial.print("Resposta do servidor: ");
      Serial.println(response);
    }
  }

  http.end();
}

