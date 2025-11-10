// ==========================================
// WIFI MANAGER - CONFIGURAÇÃO VIA WEB
// ==========================================

#include <WebServer.h>
#include <Preferences.h>

Preferences preferences;
WebServer server(80);

String savedSSID = "";
String savedPASSWORD = "";

// Página HTML para configuração Wi-Fi
void handleRoot() {
  Serial.println("Escaneando redes Wi-Fi...");
  int n = WiFi.scanNetworks();
  String options = "";

  if (n == 0) {
    options = "<option>Nenhuma rede encontrada</option>";
  } else {
    for (int i = 0; i < n; i++) {
      options += "<option value='" + WiFi.SSID(i) + "'>" + WiFi.SSID(i) + " (" + String(WiFi.RSSI(i)) + " dBm)</option>";
    }
  }

  String htmlPage = R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
    <title>Configurar WiFi</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
      body { font-family: Arial; text-align: center; margin-top: 30px; background: #f5f5f5; }
      .container { max-width: 400px; margin: 0 auto; background: white; padding: 30px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
      h2 { color: #333; }
      select, input { margin: 8px; padding: 10px; width: 100%; box-sizing: border-box; border: 1px solid #ddd; border-radius: 5px; }
      button { padding: 12px 30px; background: #007BFF; color: white; border: none; border-radius: 5px; cursor: pointer; font-size: 16px; }
      button:hover { background: #0056b3; }
      label { display: block; margin-top: 15px; text-align: left; font-weight: bold; }
    </style>
  </head>
  <body>
    <div class="container">
      <h2>⚡ Configuração Wi-Fi</h2>
      <p>Escolha sua rede Wi-Fi e insira a senha:</p>
      <form action="/save" method="POST">
        <label>Selecione sua rede:</label>
        <select name="ssid">%OPTIONS%</select>
        <label>Senha Wi-Fi:</label>
        <input name="password" placeholder="Digite a senha" type="password" required>
        <button type="submit">Salvar e Conectar</button>
      </form>
    </div>
  </body>
  </html>
  )rawliteral";

  htmlPage.replace("%OPTIONS%", options);
  server.send(200, "text/html", htmlPage);
}

// Salvar credenciais Wi-Fi
void handleSave() {
  Serial.println("\n═══════════════════════════════════");
  Serial.println("  📥 REQUISIÇÃO /save RECEBIDA!");
  Serial.println("═══════════════════════════════════");
  
  // Debug: mostrar método e URI
  Serial.print("Método: ");
  Serial.println(server.method() == HTTP_POST ? "POST" : "GET");
  Serial.print("URI: ");
  Serial.println(server.uri());
  Serial.print("Número de argumentos: ");
  Serial.println(server.args());
  
  // Mostrar todos os argumentos
  for (int i = 0; i < server.args(); i++) {
    Serial.print("  ");
    Serial.print(server.argName(i));
    Serial.print(" = ");
    if (server.argName(i) == "password") {
      Serial.println("***");
    } else {
      Serial.println(server.arg(i));
    }
  }
  
  String ssid = server.arg("ssid");
  String password = server.arg("password");
  
  Serial.print("SSID extraído: '");
  Serial.print(ssid);
  Serial.println("'");
  Serial.print("Password: ");
  Serial.println(password.length() > 0 ? "***" : "(vazia)");

  if (ssid.length() == 0) {
    Serial.println("❌ ERRO: SSID vazio!");
    String errorMsg = "<html><head><meta name='viewport' content='width=device-width, initial-scale=1'><style>body{font-family:Arial;text-align:center;margin-top:50px;background:#f5f5f5;} .container{max-width:400px;margin:0 auto;background:white;padding:30px;border-radius:10px;box-shadow:0 2px 10px rgba(0,0,0,0.1);} h2{color:#dc3545;}</style></head><body><div class='container'><h2>❌ Erro</h2><p>SSID não pode estar vazio!</p><p><a href='/'>Voltar</a></p></div></body></html>";
    server.send(400, "text/html", errorMsg);
    return;
  }

  Serial.println("💾 Salvando credenciais...");
  preferences.begin("wifi-config", false);
  preferences.putString("ssid", ssid);
  preferences.putString("password", password);
  preferences.end();
  Serial.println("✅ Credenciais salvas na memória!");

  String msg = "<html><head><meta name='viewport' content='width=device-width, initial-scale=1'><style>body{font-family:Arial;text-align:center;margin-top:50px;background:#f5f5f5;} .container{max-width:400px;margin:0 auto;background:white;padding:30px;border-radius:10px;box-shadow:0 2px 10px rgba(0,0,0,0.1);} h2{color:#28a745;}</style></head><body><div class='container'><h2>✅ Wi-Fi salvo!</h2><p><strong>Rede:</strong> " + ssid + "</p><p>O ESP32 será reiniciado para conectar...</p><p style='color:#666;font-size:14px;'>Aguarde alguns segundos.</p></div></body></html>";
  server.send(200, "text/html", msg);
  Serial.println("✅ Resposta enviada ao cliente");
  
  Serial.println("\n✅ Credenciais salvas!");
  Serial.print("Rede: ");
  Serial.println(ssid);
  Serial.println("Reiniciando ESP32 em 2 segundos...");
  
  delay(2000);
  ESP.restart();
}

// Carregar credenciais salvas
void loadWiFiCredentials() {
  preferences.begin("wifi-config", true);
  savedSSID = preferences.getString("ssid", "");
  savedPASSWORD = preferences.getString("password", "");
  preferences.end();
  
  if (savedSSID != "") {
    Serial.print("Credenciais encontradas - Rede: ");
    Serial.println(savedSSID);
  } else {
    Serial.println("Nenhuma credencial salva encontrada.");
  }
}

// Conectar ao Wi-Fi usando credenciais salvas
bool connectToWiFi() {
  if (savedSSID == "") {
    return false;
  }
  
  Serial.print("Conectando à rede: ");
  Serial.println(savedSSID);
  WiFi.begin(savedSSID.c_str(), savedPASSWORD.c_str());

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ Conectado ao Wi-Fi!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    return true;
  }

  Serial.println("\n❌ Falha ao conectar à rede salva.");
  return false;
}

// Iniciar modo Access Point para configuração
void startAccessPoint() {
  Serial.println("\n🔹 Iniciando modo Access Point...");
  WiFi.softAP("ESP32_Config", "12345678");
  
  IPAddress IP = WiFi.softAPIP();
  
  Serial.println("═══════════════════════════════════");
  Serial.println("  📶 MODO CONFIGURAÇÃO ATIVO");
  Serial.println("═══════════════════════════════════");
  Serial.println("Conecte-se à rede: ESP32_Config");
  Serial.println("Senha: 12345678");
  Serial.print("IP do Access Point: ");
  Serial.println(IP);
  Serial.println("Acesse: http://192.168.4.1");
  Serial.println("═══════════════════════════════════");

  server.on("/", handleRoot);
  server.on("/save", HTTP_POST, handleSave);
  
  // Handler para debug de requisições não encontradas
  server.onNotFound([]() {
    Serial.print("⚠️  Requisição não encontrada: ");
    Serial.println(server.uri());
    server.send(404, "text/plain", "Not Found");
  });
  
  server.begin();
  Serial.println("✅ Servidor web iniciado!");
  Serial.println("✅ Rotas configuradas:");
  Serial.println("   GET  /");
  Serial.println("   POST /save");
}

// Processar requisições do servidor web (chamar no loop)
void handleWiFiManager() {
  server.handleClient();
}

