# 🔄 Como Resetar o ESP32 para Testar Configuração WiFi

Este guia mostra como resetar as credenciais WiFi do ESP32 para testar o fluxo de configuração desde o início.

## 🚀 Métodos de Reset

### Método 1: Comando Serial (Recomendado)

1. **Abra o Serial Monitor** (115200 baud)
2. **Envie o comando**:
   ```
   resetwifi
   ```
3. O ESP32 irá:
   - Limpar as credenciais WiFi salvas
   - Reiniciar automaticamente
   - Entrar em modo Access Point (AP)

### Método 2: Reset Completo (WiFi + Energia)

Para resetar tudo (WiFi e energia acumulada):

```
resetall
```

### Método 3: Reset Manual via Código

Se preferir fazer manualmente, você pode:

1. Abrir o Serial Monitor
2. Enviar: `resetwifi`
3. Aguardar o ESP32 reiniciar

## 📱 Fluxo Após Reset

Após executar `resetwifi`, o ESP32 irá:

1. ✅ Limpar credenciais WiFi salvas
2. 🔄 Reiniciar automaticamente
3. 📶 Entrar em modo Access Point
4. 🌐 Criar rede: **ESP32_Config** (senha: **12345678**)
5. 💻 Servidor web disponível em: **http://192.168.4.1**

## 🔧 Configuração WiFi

Após o reset, siga estes passos:

### 1. Conectar à Rede do ESP32

- **Rede WiFi**: `ESP32_Config`
- **Senha**: `12345678`
- **IP do ESP32**: `192.168.4.1`

### 2. Acessar Página de Configuração

Abra o navegador e acesse:
```
http://192.168.4.1
```

### 3. Configurar WiFi

1. Selecione sua rede WiFi na lista
2. Digite a senha da rede
3. Clique em "Salvar e Conectar"

### 4. Aguardar Conexão

O ESP32 irá:
- Salvar as credenciais
- Reiniciar automaticamente
- Conectar à rede WiFi configurada
- Mostrar o IP na Serial

## 📋 Comandos Disponíveis

| Comando | Descrição |
|---------|-----------|
| `resetwifi` | Reseta credenciais WiFi e reinicia |
| `resetall` | Reseta WiFi + Energia acumulada |
| `wifi` | Mostra status atual do WiFi |
| `status` | Mostra status geral do sistema |

## 🧪 Testando o Fluxo Completo

### Passo a Passo:

1. **Resetar WiFi**:
   ```
   resetwifi
   ```

2. **Aguardar reinicialização** (2 segundos)

3. **Verificar no Serial Monitor**:
   ```
   🔹 Iniciando modo Access Point...
   ═══════════════════════════════════
     📶 MODO CONFIGURAÇÃO ATIVO
   ═══════════════════════════════════
   Conecte-se à rede: ESP32_Config
   Senha: 12345678
   Acesse: http://192.168.4.1
   ```

4. **Conectar ao WiFi do ESP32** no celular/computador

5. **Acessar** `http://192.168.4.1` no navegador

6. **Selecionar rede** e **inserir senha**

7. **Salvar** e aguardar reinicialização

8. **Verificar conexão** no Serial Monitor:
   ```
   ✅ Conectado ao Wi-Fi!
   IP: 192.168.1.xxx
   ```

## ⚠️ Troubleshooting

### ESP32 não entra em modo AP após reset

- Verifique se o comando foi enviado corretamente
- Aguarde alguns segundos após o reset
- Verifique o Serial Monitor para mensagens de erro

### Não consigo acessar 192.168.4.1

- Certifique-se de estar conectado à rede `ESP32_Config`
- Tente desconectar e reconectar
- Verifique se o firewall não está bloqueando

### Credenciais não são salvas

- Verifique se a senha WiFi está correta
- Certifique-se de que a rede está no alcance
- Veja os logs no Serial Monitor

## 💡 Dicas

- Use `wifi` para verificar o status atual sem resetar
- O comando `resetwifi` reinicia automaticamente (não precisa resetar manualmente)
- Após configurar, o ESP32 salva as credenciais permanentemente
- Para testar novamente, basta executar `resetwifi` novamente

## 🔍 Verificando Credenciais Salvas

Para verificar se há credenciais salvas, observe a mensagem no Serial ao iniciar:

**Com credenciais:**
```
Credenciais encontradas - Rede: MinhaRede
Conectando à rede: MinhaRede
✅ Conectado ao Wi-Fi!
```

**Sem credenciais:**
```
Nenhuma credencial salva encontrada.
🔹 Iniciando modo Access Point...
```

## 📝 Notas

- As credenciais são salvas em **Preferences** (não EEPROM)
- O namespace usado é `"wifi-config"`
- O reset limpa apenas as credenciais WiFi, não afeta a energia acumulada
- Use `resetall` se quiser resetar tudo

