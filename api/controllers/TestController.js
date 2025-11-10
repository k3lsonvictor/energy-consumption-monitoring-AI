import { ConsumoService } from "../services/consumo/ConsumoService.js";
import { AIAgentFactory } from "../services/ai/AIAgentFactory.js";
import { MessageParser } from "../utils/messageParser.js";

/**
 * Controlador para testes do agente de IA
 * Permite testar o fluxo completo sem precisar do Chatwoot
 */
export class TestController {
  constructor() {
    this.consumoService = new ConsumoService();
    this.aiAgent = AIAgentFactory.create();
  }

  /**
   * Testa o fluxo completo: recebe mensagem, busca dados e gera resposta
   * POST /test/chat
   * Body: { mensagem: "Qual o consumo hoje?" }
   */
  async testarChat(req, res) {
    try {
      const { mensagem, deviceId, periodo } = req.body;

      if (!mensagem) {
        return res.status(400).json({ 
          error: "Campo 'mensagem' é obrigatório",
          exemplo: {
            mensagem: "Qual o consumo hoje?",
            deviceId: 1, // opcional
            periodo: "hoje" // opcional: hoje, semana, mes, total
          }
        });
      }

      console.log("🧪 Teste - Mensagem recebida:", mensagem);

      // Extrai informações da mensagem (ou usa as fornecidas)
      const parsed = MessageParser.parse(mensagem);
      const deviceIdFinal = deviceId || parsed.deviceId;
      const periodoFinal = periodo || parsed.periodo;

      console.log("📊 Buscando dados - Device ID:", deviceIdFinal, "| Período:", periodoFinal);

      // Busca dados de consumo
      const dadosConsumo = await this.consumoService.buscarDadosConsumo(
        deviceIdFinal, 
        periodoFinal
      );

      console.log("✅ Dados encontrados:", JSON.stringify(dadosConsumo, null, 2));

      // Gera resposta com IA
      console.log("🤖 Gerando resposta com IA...");
      const respostaIA = await this.aiAgent.generateResponse(
        mensagem,
        { dadosConsumo }
      );

      console.log("💬 Resposta gerada:", respostaIA);

      // Retorna resposta completa para análise
      res.status(200).json({ 
        success: true,
        entrada: {
          mensagem,
          deviceId: deviceIdFinal,
          periodo: periodoFinal,
        },
        dadosConsumo,
        resposta: respostaIA,
        timestamp: new Date().toISOString()
      });

    } catch (error) {
      console.error("❌ Erro no teste:", error);
      res.status(500).json({ 
        error: "Erro ao processar teste",
        details: error.message,
        stack: process.env.NODE_ENV === "development" ? error.stack : undefined
      });
    }
  }

  /**
   * Lista dispositivos disponíveis para teste
   * GET /test/devices
   */
  async listarDispositivos(req, res) {
    try {
      const { PrismaClient } = await import("@prisma/client");
      const prisma = new PrismaClient();
      
      const devices = await prisma.device.findMany({
        include: {
          _count: {
            select: { readings: true }
          }
        }
      });

      res.json({
        dispositivos: devices.map(d => ({
          id: d.id,
          nome: d.name,
          porta: d.port,
          descricao: d.description,
          quantidadeLeituras: d._count.readings
        })),
        total: devices.length
      });
    } catch (error) {
      res.status(500).json({ error: error.message });
    }
  }

  /**
   * Permite trocar o agente de IA em runtime (útil para testes)
   */
  setAIAgent(agent) {
    this.aiAgent = agent;
  }
}

