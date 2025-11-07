import OpenAI from "openai";
import { AIAgent } from "./AIAgent.js";
import { config } from "../../config/env.js";

/**
 * Implementação do agente OpenAI
 */
export class OpenAIAgent extends AIAgent {
  constructor() {
    super();
    this.client = new OpenAI({
      apiKey: config.openai.apiKey,
    });
    this.model = config.ai.model;
    this.maxTokens = config.ai.maxTokens;
    this.temperature = config.ai.temperature;
  }

  async generateResponse(userMessage, context = {}, systemPrompt = null) {
    const defaultSystemPrompt = 
      "Você é um assistente virtual especializado em monitoramento de consumo de energia elétrica. " +
      "Responda sempre em português brasileiro de forma amigável e clara.";

    const systemMessage = systemPrompt || defaultSystemPrompt;

    // Prepara contexto formatado
    const contextoFormatado = context.dadosConsumo
      ? `
Dados de consumo disponíveis:
${JSON.stringify(context.dadosConsumo, null, 2)}

A mensagem do usuário foi: "${userMessage}"

Responda de forma amigável, clara e em português brasileiro. Use os dados de consumo para fornecer informações precisas.
Se o usuário perguntar sobre consumo, custos, dispositivos ou estatísticas, use os dados fornecidos acima.
Mantenha a resposta concisa e útil, entre 2-4 frases.
`
      : userMessage;

    try {
      const completion = await this.client.chat.completions.create({
        model: this.model,
        messages: [
          {
            role: "system",
            content: systemMessage,
          },
          {
            role: "user",
            content: contextoFormatado,
          },
        ],
        max_tokens: this.maxTokens,
        temperature: this.temperature,
      });

      return completion.choices[0].message.content;
    } catch (error) {
      console.error("Erro ao gerar resposta com OpenAI:", error);
      throw new Error(`Erro ao gerar resposta: ${error.message}`);
    }
  }

  async validate() {
    try {
      // Testa se a API key é válida fazendo uma requisição simples
      await this.client.models.list();
      return true;
    } catch (error) {
      console.error("Erro ao validar OpenAI:", error);
      return false;
    }
  }
}

