import { PrismaClient } from "@prisma/client";

const prisma = new PrismaClient();

/**
 * Controlador para operações com leituras de potência
 * Armazena a potência ativa (realPower) em Watts
 */
export class PowerController {
  /**
   * Cria uma nova leitura de potência ativa
   * @param {number} realPower - Potência ativa em Watts (P = média de v(t) * i(t))
   */
  async criar(req, res) {
    try {
      const { port, realPower, powerW } = req.body;
      
      console.log("⚡ POWER - Recebido leitura de potência ativa:", { port, realPower, powerW, body: req.body });
    
      // Validação dos campos obrigatórios
      if (!port) {
        return res.status(400).json({ error: "Campo 'port' é obrigatório" });
      }
      
      // Usar realPower (potência ativa) se fornecido, caso contrário usar powerW
      const powerValue = realPower !== undefined ? realPower : powerW;
      
      if (powerValue === undefined || powerValue === null) {
        return res.status(400).json({ error: "Campo 'realPower' ou 'powerW' é obrigatório" });
      }
    
      // Encontra o dispositivo pela porta
      const device = await prisma.device.findUnique({ 
        where: { port: String(port) } 
      });
      
      if (!device) {
        return res.status(404).json({ 
          error: `Nenhum dispositivo cadastrado na porta ${port}` 
        });
      }
    
      // Cria leitura de potência ativa associada ao dispositivo
      // powerW armazena a potência ativa em Watts
      const powerReading = await prisma.powerReading.create({
        data: {
          deviceId: device.id,
          powerW: Number(powerValue), // Potência ativa em Watts
        },
      });
    
      console.log("✅ POWER - Leitura de potência salva:", powerReading);
    
      res.json({ 
        message: `Leitura de potência registrada para ${device.name}`, 
        powerReading 
      });
    } catch (error) {
      console.error("Erro ao processar leitura de potência:", error);
      res.status(500).json({ 
        error: "Erro interno do servidor", 
        details: error.message 
      });
    }
  }

  /**
   * Lista leituras de potência de um dispositivo
   */
  async listarPorDevice(req, res) {
    try {
      const { deviceId } = req.params;
      
      const powerReadings = await prisma.powerReading.findMany({
        where: { deviceId: parseInt(deviceId) },
        orderBy: { createdAt: 'desc' },
        take: 100, // Limitar a 100 últimas leituras
      });
      
      res.json({ powerReadings });
    } catch (error) {
      console.error("Erro ao listar leituras de potência:", error);
      res.status(500).json({ 
        error: "Erro interno do servidor", 
        details: error.message 
      });
    }
  }
}

