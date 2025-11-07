import { ChatwootService } from "../services/chatwoot/ChatwootService.js";
import { ConsumoService } from "../services/consumo/ConsumoService.js";
import { AIAgentFactory } from "../services/ai/AIAgentFactory.js";
import { MessageParser } from "../utils/messageParser.js";

/**
 * Controlador para webhooks do Chatwoot
 */
export class WebhookController {
  constructor() {
    this.chatwootService = new ChatwootService();
    this.consumoService = new ConsumoService();
    // O agente de IA é criado dinamicamente via factory
    this.aiAgent = AIAgentFactory.create();
  }

  /**
   * Processa webhook do Chatwoot
   */
  async processar(req, res) {
    try {
      // Extrai dados do webhook
      const dados = this.chatwootService.extrairDadosWebhook(req.body);

      // Verifica se deve processar
      if (!this.chatwootService.deveProcessar(dados)) {
        return res.status(200).json({ received: true });
      }

      const { mensagemUsuario, conversationId } = dados;

      console.log("Mensagem recebida:", mensagemUsuario, "| Conversation ID:", conversationId);

      // Extrai informações da mensagem
      const { deviceId, periodo } = MessageParser.parse(mensagemUsuario);

      // Busca dados de consumo
      const dadosConsumo = await this.consumoService.buscarDadosConsumo(
        deviceId, 
        periodo
      );

      // Gera resposta com IA
      const respostaIA = await this.aiAgent.generateResponse(
        mensagemUsuario,
        { dadosConsumo }
      );

      // Envia resposta via Chatwoot
      await this.chatwootService.enviarMensagem(conversationId, respostaIA);

      res.status(200).json({ 
        success: true, 
        message: "Resposta enviada com sucesso",
        resposta: respostaIA 
      });

    } catch (error) {
      console.error("Erro no webhook Chatwoot:", error);
      
      // Tenta enviar mensagem de erro
      try {
        const dados = this.chatwootService.extrairDadosWebhook(req.body);
        if (dados.conversationId) {
          await this.chatwootService.enviarMensagem(
            dados.conversationId,
            "Desculpe, ocorreu um erro ao processar sua solicitação. Por favor, tente novamente."
          );
        }
      } catch (err) {
        console.error("Erro ao enviar mensagem de erro:", err);
      }

      res.status(500).json({ 
        error: "Erro ao processar mensagem",
        details: error.message 
      });
    }
  }

  /**
   * Permite trocar o agente de IA em runtime (útil para testes)
   */
  setAIAgent(agent) {
    this.aiAgent = agent;
  }
}

