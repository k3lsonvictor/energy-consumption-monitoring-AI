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
      const { port, energyWh, durationMin } = req.body;
      
      console.log("Recebido:", { port, energyWh, durationMin, body: req.body });
    
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

