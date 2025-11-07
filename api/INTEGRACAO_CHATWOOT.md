# Integração Chatwoot + OpenAI para Consulta de Consumo

Este documento explica como configurar e usar a integração entre Chatwoot, OpenAI e a API de monitoramento de consumo de energia.

## 📋 Pré-requisitos

1. Conta no Chatwoot (https://www.chatwoot.com)
2. Chave de API da OpenAI (https://platform.openai.com/api-keys)
3. Node.js instalado
4. Banco de dados configurado (Prisma)

## 🔧 Configuração

### 1. Instalar Dependências

```bash
cd api
npm install
```

### 2. Configurar Variáveis de Ambiente

Crie um arquivo `.env` na pasta `api/` com as seguintes variáveis:

```env
# OpenAI Configuration
OPENAI_API_KEY=sk-your-openai-api-key-here

# Chatwoot Configuration
CHATWOOT_BASE_URL=https://app.chatwoot.com
CHATWOOT_ACCESS_TOKEN=your-chatwoot-access-token
CHATWOOT_ACCOUNT_ID=your-chatwoot-account-id
```

### 3. Obter Credenciais do Chatwoot

1. **Access Token:**
   - Acesse seu Chatwoot
   - Vá em Settings > Applications > API Tokens
   - Crie um novo token com permissões de leitura/escrita de mensagens

2. **Account ID:**
   - Acesse Settings > Accounts
   - O ID da conta está na URL ou no painel

3. **Base URL:**
   - Se usar Chatwoot Cloud: `https://app.chatwoot.com`
   - Se usar self-hosted: `https://seu-dominio.com`

### 4. Configurar Webhook no Chatwoot

1. Acesse Settings > Applications > Webhooks
2. Crie um novo webhook apontando para:
   ```
   https://seu-servidor.com/webhook/chatwoot
   ```
3. Selecione os eventos:
   - `message_created`
   - `message.updated` (opcional)

## 🚀 Como Funciona

1. **Usuário envia mensagem via WhatsApp** → Chatwoot recebe
2. **Chatwoot envia webhook** → Sua API recebe em `/webhook/chatwoot`
3. **API extrai informações** da mensagem (dispositivo, período)
4. **API busca dados** de consumo no banco de dados
5. **API envia contexto** para OpenAI gerar resposta personalizada
6. **API envia resposta** de volta via Chatwoot → WhatsApp

## 💬 Exemplos de Mensagens

Os usuários podem enviar mensagens como:

- "Qual o consumo hoje?"
- "Quanto gastou o dispositivo 1 esta semana?"
- "Mostre o consumo total"
- "Consumo do dispositivo 2 no mês passado"
- "Quanto custou a energia hoje?"

A IA irá:
- Identificar automaticamente o dispositivo (se mencionado)
- Identificar o período (hoje, semana, mês, total)
- Buscar dados relevantes
- Gerar resposta natural e personalizada

## 📊 Endpoint de Teste

Você pode testar a busca de dados diretamente:

```bash
# Todos os dispositivos, período total
GET http://localhost:3000/consumo

# Dispositivo específico
GET http://localhost:3000/consumo?deviceId=1

# Período específico
GET http://localhost:3000/consumo?periodo=hoje
GET http://localhost:3000/consumo?periodo=semana
GET http://localhost:3000/consumo?periodo=mes
```

## 🔍 Estrutura da Resposta

A função `buscarDadosConsumo` retorna:

```json
{
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
}
```

## 🛠️ Troubleshooting

### Webhook não está recebendo mensagens

1. Verifique se o webhook está configurado corretamente no Chatwoot
2. Confirme que a URL está acessível publicamente (use ngrok para desenvolvimento)
3. Verifique os logs da API: `console.log` mostrará as mensagens recebidas

### OpenAI não responde

1. Verifique se a chave de API está correta
2. Confirme se há créditos disponíveis na conta OpenAI
3. Verifique os logs para erros específicos

### Chatwoot não envia mensagens

1. Verifique se o `CHATWOOT_ACCESS_TOKEN` está correto
2. Confirme se o `CHATWOOT_ACCOUNT_ID` está correto
3. Verifique se o token tem permissões adequadas

## 📝 Notas

- A API usa GPT-3.5-turbo por padrão (mais econômico)
- Respostas são limitadas a 200 tokens para manter concisão
- O sistema identifica automaticamente dispositivos e períodos nas mensagens
- Se não identificar dispositivo, retorna dados de todos os dispositivos

## 🔒 Segurança

- Use variáveis de ambiente para credenciais
- Configure HTTPS em produção
- Valide e sanitize inputs
- Implemente rate limiting para webhooks
- Use autenticação adicional se necessário

