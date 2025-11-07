// Configurações de ambiente
export const config = {
  openai: {
    apiKey: process.env.OPENAI_API_KEY || "sk-your-api-key-here",
  },
  chatwoot: {
    baseUrl: process.env.CHATWOOT_BASE_URL || "https://app.chatwoot.com",
    accessToken: process.env.CHATWOOT_ACCESS_TOKEN || "your-access-token",
    accountId: process.env.CHATWOOT_ACCOUNT_ID || "your-account-id",
  },
  ai: {
    provider: process.env.AI_PROVIDER || "openai", // openai, anthropic, etc
    model: process.env.AI_MODEL || "gpt-3.5-turbo",
    maxTokens: parseInt(process.env.AI_MAX_TOKENS || "200"),
    temperature: parseFloat(process.env.AI_TEMPERATURE || "0.7"),
  },
};

