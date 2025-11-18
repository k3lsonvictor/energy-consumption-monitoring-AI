import { PrismaClient } from "@prisma/client";

const prisma = new PrismaClient();

/**
 * Controlador para operações com leituras
 */
export class ReadingController {
  /**
   * Cria uma nova leitura
   */
  async criar(req, res) {
    try {
      const { port, energyWh, durationMin, realPower } = req.body;
      
      console.log("📊 READING - Recebido:", { port, energyWh, durationMin, realPower, body: req.body });
    
      // Validação dos campos obrigatórios
      if (!port) {
        return res.status(400).json({ error: "Campo 'port' é obrigatório" });
      }
      if (energyWh === undefined || energyWh === null) {
        return res.status(400).json({ error: "Campo 'energyWh' é obrigatório" });
      }
      if (durationMin === undefined || durationMin === null) {
        return res.status(400).json({ error: "Campo 'durationMin' é obrigatório" });
      }
      
      // Rejeitar leituras que parecem ser de potência (energyWh = 0 e durationMin = 5)
      // Essas devem ir para /power, não para /readings
      if (energyWh === 0 && durationMin === 5 && realPower !== undefined) {
        return res.status(400).json({ 
          error: "Esta parece ser uma leitura de potência. Use o endpoint /power em vez de /readings",
          hint: "Envie para POST /power com { port, realPower }"
        });
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
    
      // Cria leitura associada ao dispositivo
      const reading = await prisma.reading.create({
        data: {
          deviceId: device.id,
          energyWh: Number(energyWh),
          durationMin: Number(durationMin),
        },
      });
    
      res.json({ 
        message: `Leitura registrada para ${device.name}`, 
        reading 
      });
    } catch (error) {
      console.error("Erro ao processar leitura:", error);
      res.status(500).json({ 
        error: "Erro interno do servidor", 
        details: error.message 
      });
    }
  }
}

