# 🧪 Guia de Teste do Agente de IA

Este guia mostra como testar o fluxo completo de respostas do agente de IA usando os dados reais do banco de dados, sem precisar integrar com WhatsApp.

## 🚀 Endpoints de Teste

### 1. Testar Chat Completo

**POST** `/test/chat`

Testa o fluxo completo: recebe mensagem, busca dados do banco e gera resposta com IA.

#### Exemplo de Requisição:

```bash
curl -X POST http://localhost:3000/test/chat \
  -H "Content-Type: application/json" \
  -d '{
    "mensagem": "Qual o consumo hoje?"
  }'
```

#### Exemplo com Parâmetros Opcionais:

```bash
curl -X POST http://localhost:3000/test/chat \
  -H "Content-Type: application/json" \
  -d '{
    "mensagem": "Quanto gastou o dispositivo 1 esta semana?",
    "deviceId": 1,
    "periodo": "semana"
  }'
```

#### Resposta:

```json
{
  "success": true,
  "entrada": {
    "mensagem": "Qual o consumo hoje?",
    "deviceId": null,
    "periodo": "hoje"
  },
  "dadosConsumo": {
    "periodo": "hoje",
    "resumo": {
      "totalWh": "1500.00",
      "totalKWh": "1.50",
      "custoTotal": "1.43"
    },
    "dispositivos": [
      {
        "dispositivo": "Geladeira",
        "porta": "A0",
        "totalWh": "800.00",
        "totalKWh": "0.80",
        "custoEstimado": "0.76",
        "quantidadeLeituras": 10,
        "tempoTotalMinutos": 1440,
        "ultimaLeitura": "2024-01-15T10:30:00Z"
      }
    ]
  },
  "resposta": "Hoje o consumo total foi de 1.50 kWh, com um custo estimado de R$ 1.43. A geladeira foi o dispositivo que mais consumiu, com 0.80 kWh.",
  "timestamp": "2024-01-15T12:00:00.000Z"
}
```

### 2. Listar Dispositivos Disponíveis

**GET** `/test/devices`

Lista todos os dispositivos cadastrados para facilitar os testes.

#### Exemplo:

```bash
curl http://localhost:3000/test/devices
```

#### Resposta:

```json
{
  "dispositivos": [
    {
      "id": 1,
      "nome": "Geladeira",
      "porta": "A0",
      "descricao": "Geladeira da cozinha",
      "quantidadeLeituras": 25
    },
    {
      "id": 2,
      "nome": "Ar Condicionado",
      "porta": "A1",
      "descricao": "Ar do quarto",
      "quantidadeLeituras": 15
    }
  ],
  "total": 2
}
```

## 📝 Exemplos de Mensagens para Testar

### Consumo Geral

```json
{ "mensagem": "Qual o consumo hoje?" }
{ "mensagem": "Quanto gastamos esta semana?" }
{ "mensagem": "Mostre o consumo do mês" }
{ "mensagem": "Qual o consumo total?" }
```

### Consumo por Dispositivo

```json
{ "mensagem": "Quanto consumiu o dispositivo 1 hoje?" }
{ "mensagem": "Consumo da geladeira esta semana" }
{ "mensagem": "Dispositivo 2, consumo do mês" }
```

### Custos

```json
{ "mensagem": "Quanto custou a energia hoje?" }
{ "mensagem": "Qual o custo estimado desta semana?" }
{ "mensagem": "Quanto gastamos no mês?" }
```

### Comparações

```json
{ "mensagem": "Qual dispositivo consome mais?" }
{ "mensagem": "Compare o consumo de hoje com a semana passada" }
```

## 🧪 Testando com Postman/Insomnia

### 1. Criar Nova Requisição

- **Método**: POST
- **URL**: `http://localhost:3000/test/chat`
- **Headers**: 
  - `Content-Type: application/json`

### 2. Body (JSON)

```json
{
  "mensagem": "Qual o consumo hoje?"
}
```

### 3. Enviar e Ver Resposta

A resposta incluirá:
- A mensagem original
- Os dados de consumo encontrados
- A resposta gerada pela IA

## 🧪 Testando com cURL

### Teste Básico

```bash
curl -X POST http://localhost:3000/test/chat \
  -H "Content-Type: application/json" \
  -d '{"mensagem": "Qual o consumo hoje?"}'
```

### Teste com Dispositivo Específico

```bash
curl -X POST http://localhost:3000/test/chat \
  -H "Content-Type: application/json" \
  -d '{
    "mensagem": "Quanto consumiu o dispositivo 1?",
    "deviceId": 1
  }'
```

### Teste com Período Específico

```bash
curl -X POST http://localhost:3000/test/chat \
  -H "Content-Type: application/json" \
  -d '{
    "mensagem": "Consumo da semana",
    "periodo": "semana"
  }'
```

## 🧪 Testando com Node.js

Crie um arquivo `test-agent.js`:

```javascript
import fetch from 'node-fetch';

async function testarAgente(mensagem, deviceId = null, periodo = null) {
  const response = await fetch('http://localhost:3000/test/chat', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    body: JSON.stringify({
      mensagem,
      deviceId,
      periodo
    }),
  });

  const data = await response.json();
  console.log('📨 Mensagem:', mensagem);
  console.log('🤖 Resposta:', data.resposta);
  console.log('📊 Dados:', JSON.stringify(data.dadosConsumo, null, 2));
  return data;
}

// Testes
await testarAgente('Qual o consumo hoje?');
await testarAgente('Quanto gastou o dispositivo 1?', 1);
await testarAgente('Consumo da semana', null, 'semana');
```

Execute:
```bash
node test-agent.js
```

## 🔍 Verificando os Logs

O endpoint de teste gera logs detalhados no console:

```
🧪 Teste - Mensagem recebida: Qual o consumo hoje?
📊 Buscando dados - Device ID: null | Período: hoje
✅ Dados encontrados: {...}
🤖 Gerando resposta com IA...
💬 Resposta gerada: Hoje o consumo total foi...
```

## ⚠️ Troubleshooting

### Erro: "Campo 'mensagem' é obrigatório"

Certifique-se de enviar o campo `mensagem` no body da requisição.

### Erro: "Erro ao gerar resposta"

1. Verifique se a chave da OpenAI está configurada no `.env`
2. Verifique se há créditos na conta OpenAI
3. Veja os logs do servidor para mais detalhes

### Nenhum dado encontrado

1. Verifique se há dispositivos cadastrados: `GET /test/devices`
2. Verifique se há leituras registradas: `GET /devices/:id/readings`
3. Tente um período diferente (ex: "total" em vez de "hoje")

## 📊 Estrutura da Resposta

A resposta sempre inclui:

- **entrada**: A mensagem e parâmetros recebidos
- **dadosConsumo**: Os dados encontrados no banco
  - `periodo`: Período consultado
  - `resumo`: Totais gerais
  - `dispositivos`: Dados por dispositivo
- **resposta**: A resposta gerada pela IA
- **timestamp**: Quando a resposta foi gerada

Isso permite analisar todo o fluxo e verificar se os dados estão sendo processados corretamente!

