// ==========================================
// FUNÇÕES HTTP
// ==========================================

// Enviar dados para o servidor
void sendDataToServer(float energy, float duration) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠️  WiFi desconectado! Não é possível salvar no banco.");
    return;
  }

  HTTPClient http;
  http.setTimeout(10000); // Timeout de 10 segundos
  http.begin(serverUrl);
  http.addHeader("Content-Type", "application/json");

  String port = "1";
  
  String jsonPayload = "{\"port\": \"" + port + "\"" +
                       ", \"energyWh\": " + String(energy, 6) +
                       ", \"durationMin\": " + String(duration) + "}";

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

