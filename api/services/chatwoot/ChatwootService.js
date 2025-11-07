import axios from "axios";
import { config } from "../../config/env.js";

/**
 * Serviço para interagir com a API do Chatwoot
 */
export class ChatwootService {
  constructor() {
    this.baseUrl = config.chatwoot.baseUrl;
    this.accessToken = config.chatwoot.accessToken;
    this.accountId = config.chatwoot.accountId;
  }

  /**
   * Envia mensagem para uma conversa no Chatwoot
   * @param {number|string} conversationId - ID da conversa
   * @param {string} message - Conteúdo da mensagem
   * @returns {Promise<object>} Resposta da API
   */
  async enviarMensagem(conversationId, message) {
    try {
      const response = await axios.post(
        `${this.baseUrl}/public/api/v1/accounts/${this.accountId}/conversations/${conversationId}/messages`,
        {
          content: message,
          message_type: "outgoing",
          private: false,
        },
        {
          headers: {
            "api_access_token": this.accessToken,
            "Content-Type": "application/json",
          },
        }
      );
      return response.data;
    } catch (error) {
      console.error("Erro ao enviar mensagem via Chatwoot:", error.response?.data || error.message);
      throw new Error(`Erro ao enviar mensagem: ${error.message}`);
    }
  }

  /**
   * Extrai informações do payload do webhook
   * @param {object} payload - Payload do webhook
   * @returns {object} Dados extraídos
   */
  extrairDadosWebhook(payload) {
    const { event, conversation, message, payload: altPayload } = payload;
    
    // Suporta diferentes formatos de webhook
    const messageData = message || altPayload?.message;
    const conversationData = conversation || altPayload?.conversation;
    
    const messageType = messageData?.message_type || messageData?.inbox?.channel_type;
    const mensagemUsuario = messageData?.content || messageData?.text || "";
    const conversationId = conversationData?.id || conversationData?.conversation_id;

    return {
      event,
      messageType,
      mensagemUsuario,
      conversationId,
      messageData,
      conversationData,
    };
  }

  /**
   * Verifica se o webhook deve ser processado
   * @param {object} dados - Dados extraídos do webhook
   * @returns {boolean}
   */
  deveProcessar(dados) {
    const { event, messageType, mensagemUsuario, conversationId } = dados;

    // Ignora eventos que não são de mensagem criada
    if (event !== "message_created" && event !== "message.updated") {
      if (messageType !== "incoming") {
        return false;
      }
    }

    // Ignora mensagens que não são recebidas
    if (messageType !== "incoming" && messageType !== undefined) {
      return false;
    }

    // Ignora se não tem conversation ID ou mensagem vazia
    if (!conversationId || !mensagemUsuario.trim()) {
      return false;
    }

    return true;
  }
}

