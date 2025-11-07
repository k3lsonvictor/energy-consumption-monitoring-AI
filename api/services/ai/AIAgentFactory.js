import { OpenAIAgent } from "./OpenAIAgent.js";
import { config } from "../../config/env.js";

/**
 * Factory para criar instâncias de agentes de IA
 * Facilita a troca entre diferentes provedores
 */
export class AIAgentFactory {
  static create(provider = null) {
    const providerName = provider || config.ai.provider;

    switch (providerName.toLowerCase()) {
      case "openai":
        return new OpenAIAgent();
      
      // Adicione outros provedores aqui:
      // case "anthropic":
      //   return new AnthropicAgent();
      // case "google":
      //   return new GoogleAIAgent();
      
      default:
        throw new Error(`Provedor de IA não suportado: ${providerName}`);
    }
  }

  static getAvailableProviders() {
    return ["openai"]; // Adicione outros quando implementar
  }
}

