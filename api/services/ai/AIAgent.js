// Interface abstrata para agentes de IA
export class AIAgent {
  /**
   * Gera uma resposta baseada no contexto e mensagem do usuário
   * @param {string} userMessage - Mensagem do usuário
   * @param {object} context - Contexto adicional (dados de consumo, etc)
   * @param {string} systemPrompt - Prompt do sistema (opcional)
   * @returns {Promise<string>} Resposta gerada pela IA
   */
  async generateResponse(userMessage, context = {}, systemPrompt = null) {
    throw new Error("Método generateResponse deve ser implementado");
  }

  /**
   * Valida se o agente está configurado corretamente
   * @returns {Promise<boolean>}
   */
  async validate() {
    throw new Error("Método validate deve ser implementado");
  }
}

