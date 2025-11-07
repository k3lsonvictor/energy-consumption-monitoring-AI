# Estrutura Modular da API

Esta documentação explica a estrutura modular da API e como trocar facilmente o agente de IA.

## 📁 Estrutura de Pastas

```
api/
├── config/              # Configurações
│   ├── constants.js     # Constantes da aplicação
│   └── env.js          # Variáveis de ambiente
├── services/            # Serviços de negócio
│   ├── ai/             # Agentes de IA
│   │   ├── AIAgent.js           # Interface abstrata
│   │   ├── OpenAIAgent.js       # Implementação OpenAI
│   │   ├── AIAgentFactory.js    # Factory para criar agentes
│   │   └── AnthropicAgent.js.example  # Exemplo de outro provedor
│   ├── chatwoot/        # Serviço Chatwoot
│   │   └── ChatwootService.js
│   └── consumo/         # Serviço de consumo
│       └── ConsumoService.js
├── controllers/         # Controladores (lógica de requisições)
│   ├── DeviceController.js
│   ├── ReadingController.js
│   ├── WebhookController.js
│   └── ConsumoController.js
├── routes/              # Definição de rotas
│   ├── deviceRoutes.js
│   ├── readingRoutes.js
│   ├── webhookRoutes.js
│   └── consumoRoutes.js
├── utils/               # Utilitários
│   └── messageParser.js
└── index.js            # Ponto de entrada da aplicação
```

## 🔄 Como Trocar o Agente de IA

### Opção 1: Via Variável de Ambiente (Recomendado)

Configure a variável `AI_PROVIDER` no arquivo `.env`:

```env
AI_PROVIDER=openai
# ou
AI_PROVIDER=anthropic
```

O sistema automaticamente usará o provedor configurado.

### Opção 2: Modificar o Factory

Edite `services/ai/AIAgentFactory.js` e adicione seu novo provedor:

```javascript
static create(provider = null) {
  const providerName = provider || config.ai.provider;

  switch (providerName.toLowerCase()) {
    case "openai":
      return new OpenAIAgent();
    
    case "anthropic":
      return new AnthropicAgent();  // Adicione aqui
    
    case "meu-provedor":
      return new MeuProvedorAgent();  // Seu novo provedor
    
    default:
      throw new Error(`Provedor de IA não suportado: ${providerName}`);
  }
}
```

### Opção 3: Trocar em Runtime (Para Testes)

No `WebhookController`, você pode trocar o agente:

```javascript
const webhookController = new WebhookController();
webhookController.setAIAgent(new MeuProvedorAgent());
```

## 🆕 Como Adicionar um Novo Provedor de IA

### Passo 1: Criar a Classe do Agente

Crie um novo arquivo em `services/ai/`, por exemplo `MeuProvedorAgent.js`:

```javascript
import { AIAgent } from "./AIAgent.js";
import { config } from "../../config/env.js";

export class MeuProvedorAgent extends AIAgent {
  constructor() {
    super();
    // Inicialize seu cliente aqui
  }

  async generateResponse(userMessage, context = {}, systemPrompt = null) {
    // Implemente a lógica para gerar resposta
    // userMessage: mensagem do usuário
    // context: objeto com dados de consumo (context.dadosConsumo)
    // systemPrompt: prompt do sistema (opcional)
    
    // Retorne a resposta como string
    return "Resposta gerada";
  }

  async validate() {
    // Valide se a configuração está correta
    return true;
  }
}
```

### Passo 2: Registrar no Factory

Adicione no `AIAgentFactory.js`:

```javascript
import { MeuProvedorAgent } from "./MeuProvedorAgent.js";

// No método create():
case "meu-provedor":
  return new MeuProvedorAgent();
```

### Passo 3: Atualizar Lista de Provedores

No método `getAvailableProviders()`:

```javascript
static getAvailableProviders() {
  return ["openai", "meu-provedor"];
}
```

### Passo 4: Configurar Variáveis de Ambiente

Adicione as variáveis necessárias no `.env`:

```env
AI_PROVIDER=meu-provedor
MEU_PROVIDER_API_KEY=sua-chave-aqui
```

## 📋 Interface do Agente (AIAgent)

Todos os agentes devem implementar:

```javascript
class MeuAgente extends AIAgent {
  // Gera resposta baseada na mensagem e contexto
  async generateResponse(userMessage, context = {}, systemPrompt = null) {
    // Retorna: Promise<string>
  }

  // Valida se está configurado corretamente
  async validate() {
    // Retorna: Promise<boolean>
  }
}
```

### Parâmetros de `generateResponse`:

- **userMessage** (string): Mensagem original do usuário
- **context** (object): Objeto com dados adicionais
  - `context.dadosConsumo`: Dados de consumo formatados
- **systemPrompt** (string|null): Prompt do sistema (opcional)

### Retorno:

- **Promise<string>**: Resposta gerada pela IA

## 🔧 Configurações Disponíveis

No arquivo `config/env.js`, você pode configurar:

```javascript
ai: {
  provider: "openai",        // Provedor padrão
  model: "gpt-3.5-turbo",   // Modelo a usar
  maxTokens: 200,            // Máximo de tokens
  temperature: 0.7,          // Temperatura (criatividade)
}
```

## 📝 Exemplo Completo: Adicionar Google Gemini

### 1. Instalar dependência:
```bash
npm install @google/generative-ai
```

### 2. Criar `GoogleAIAgent.js`:

```javascript
import { GoogleGenerativeAI } from "@google/generative-ai";
import { AIAgent } from "./AIAgent.js";
import { config } from "../../config/env.js";

export class GoogleAIAgent extends AIAgent {
  constructor() {
    super();
    this.genAI = new GoogleGenerativeAI(process.env.GOOGLE_AI_API_KEY);
    this.model = this.genAI.getGenerativeModel({ 
      model: config.ai.model || "gemini-pro" 
    });
  }

  async generateResponse(userMessage, context = {}, systemPrompt = null) {
    const prompt = context.dadosConsumo
      ? `${systemPrompt || ""}\n\nDados: ${JSON.stringify(context.dadosConsumo)}\n\nPergunta: ${userMessage}`
      : userMessage;

    const result = await this.model.generateContent(prompt);
    return result.response.text();
  }

  async validate() {
    try {
      await this.model.generateContent("test");
      return true;
    } catch {
      return false;
    }
  }
}
```

### 3. Adicionar no Factory:

```javascript
import { GoogleAIAgent } from "./GoogleAIAgent.js";

case "google":
  return new GoogleAIAgent();
```

### 4. Configurar:

```env
AI_PROVIDER=google
GOOGLE_AI_API_KEY=sua-chave
```

## ✅ Vantagens da Estrutura Modular

1. **Fácil troca de provedores**: Apenas mude a variável de ambiente
2. **Testabilidade**: Pode injetar mocks facilmente
3. **Manutenibilidade**: Código organizado e separado por responsabilidade
4. **Extensibilidade**: Adicionar novos provedores é simples
5. **Reutilização**: Serviços podem ser usados em diferentes contextos

## 🧪 Testando com Diferentes Agentes

```javascript
// Em um arquivo de teste
import { AIAgentFactory } from "./services/ai/AIAgentFactory.js";

// Testa OpenAI
const openaiAgent = AIAgentFactory.create("openai");
const resposta1 = await openaiAgent.generateResponse("Qual o consumo?", { dadosConsumo });

// Testa outro provedor
const outroAgent = AIAgentFactory.create("anthropic");
const resposta2 = await outroAgent.generateResponse("Qual o consumo?", { dadosConsumo });
```

