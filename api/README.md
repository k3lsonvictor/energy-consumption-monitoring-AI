# API de Monitoramento de Consumo de Energia

API modular para monitoramento de consumo de energia com integração Chatwoot + IA.

## 🚀 Início Rápido

```bash
# Instalar dependências
npm install

# Configurar variáveis de ambiente
cp .env.example .env
# Edite o .env com suas credenciais

# Executar
npm run dev
```

## 📚 Documentação

- [Integração Chatwoot](./INTEGRACAO_CHATWOOT.md) - Como configurar Chatwoot + OpenAI
- [Estrutura Modular](./ESTRUTURA_MODULAR.md) - Como trocar o agente de IA

## 🔄 Trocar Agente de IA

### Método 1: Variável de Ambiente (Mais Fácil)

```env
AI_PROVIDER=openai
```

### Método 2: Criar Novo Provedor

1. Crie `services/ai/MeuProvedorAgent.js` estendendo `AIAgent`
2. Adicione no `AIAgentFactory.js`
3. Configure `AI_PROVIDER=meu-provedor`

Veja [ESTRUTURA_MODULAR.md](./ESTRUTURA_MODULAR.md) para detalhes.

## 📁 Estrutura

```
api/
├── config/          # Configurações
├── services/        # Lógica de negócio
│   ├── ai/         # Agentes de IA (fácil trocar!)
│   ├── chatwoot/   # Serviço Chatwoot
│   └── consumo/    # Serviço de consumo
├── controllers/     # Controladores HTTP
├── routes/          # Rotas
└── utils/           # Utilitários
```

## 🔌 Endpoints

### Dispositivos
- `GET /devices` - Lista todos os dispositivos
- `POST /devices` - Cria um novo dispositivo
- `POST /devices/associar` - Associa um nome a uma entrada (pino) do ESP32
- `GET /devices/:id` - Busca dispositivo por ID
- `PUT /devices/:id` - Atualiza um dispositivo
- `GET /devices/porta/:porta` - Busca dispositivo por porta/pino
- `GET /devices/:id/readings` - Leituras do dispositivo
- `GET /devices/:id/summary` - Resumo do dispositivo

### Leituras e Consumo
- `POST /readings` - Registra leitura
- `GET /consumo` - Consulta consumo

### Webhooks
- `POST /webhook/chatwoot` - Webhook Chatwoot

### Sistema
- `GET /health` - Health check

## ⚙️ Variáveis de Ambiente

```env
# OpenAI
OPENAI_API_KEY=sk-...

# Chatwoot
CHATWOOT_BASE_URL=https://app.chatwoot.com
CHATWOOT_ACCESS_TOKEN=...
CHATWOOT_ACCOUNT_ID=...

# IA (opcional)
AI_PROVIDER=openai
AI_MODEL=gpt-3.5-turbo
AI_MAX_TOKENS=200
AI_TEMPERATURE=0.7
```

