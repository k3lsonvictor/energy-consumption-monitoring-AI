/**
 * Utilitário para extrair informações das mensagens do usuário
 */
export class MessageParser {
  /**
   * Extrai ID do dispositivo da mensagem
   * @param {string} mensagem - Mensagem do usuário
   * @returns {number|null} ID do dispositivo ou null
   */
  static extrairDeviceId(mensagem) {
    const patterns = [
      /(?:dispositivo|device|id)[:\s]+(\d+)/i,
      /dispositivo\s*(\d+)/i,
      /device\s*(\d+)/i,
    ];

    for (const pattern of patterns) {
      const match = mensagem.match(pattern);
      if (match) {
        return parseInt(match[1]);
      }
    }

    return null;
  }

  /**
   * Extrai período da mensagem
   * @param {string} mensagem - Mensagem do usuário
   * @returns {string} Período: "hoje", "semana", "mes", "total"
   */
  static extrairPeriodo(mensagem) {
    const periodoMatch = mensagem.match(/(hoje|semana|mês|mes|mês passado|mes passado|último mês|ultimo mes|total|todas?)/i);
    
    if (!periodoMatch) {
      return "total";
    }

    let periodo = periodoMatch[1].toLowerCase().replace("mês", "mes");
    
    // Normaliza período
    if (periodo.includes("passado") || periodo.includes("último") || periodo.includes("ultimo")) {
      periodo = "mes";
    }

    return periodo;
  }

  /**
   * Extrai todas as informações relevantes da mensagem
   * @param {string} mensagem - Mensagem do usuário
   * @returns {object} Informações extraídas
   */
  static parse(mensagem) {
    return {
      deviceId: this.extrairDeviceId(mensagem),
      periodo: this.extrairPeriodo(mensagem),
      mensagemOriginal: mensagem,
    };
  }
}

