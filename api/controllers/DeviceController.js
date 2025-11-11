import { PrismaClient } from "@prisma/client";
import { CUSTO_POR_KWH } from "../config/constants.js";

const prisma = new PrismaClient();

/**
 * Controlador para operações com dispositivos
 */
export class DeviceController {
  /**
   * Lista todos os dispositivos
   */
  async listar(req, res) {
    try {
      const devices = await prisma.device.findMany();
      res.json(devices);
    } catch (error) {
      res.status(500).json({ error: error.message });
    }
  }

  /**
   * Cria um novo dispositivo
   */
  async criar(req, res) {
    try {
      const { name, description, port } = req.body;
      const device = await prisma.device.create({ 
        data: { name, description, port } 
      });
      res.json(device);
    } catch (error) {
      res.status(500).json({ error: error.message });
    }
  }

  /**
   * Lista leituras de um dispositivo
   */
  async listarLeituras(req, res) {
    try {
      const { id } = req.params;
      const readings = await prisma.reading.findMany({
        where: { deviceId: Number(id) },
        orderBy: { createdAt: "asc" },
      });
      res.json(readings);
    } catch (error) {
      res.status(500).json({ error: error.message });
    }
  }

  /**
   * Calcula resumo de um dispositivo
   */
  async resumo(req, res) {
    try {
      const { id } = req.params;
      const readings = await prisma.reading.findMany({
        where: { deviceId: Number(id) },
      });

      const totalWh = readings.reduce((sum, r) => sum + r.energyWh, 0);
      const totalKWh = totalWh / 1000;
      const custoEstimado = totalKWh * CUSTO_POR_KWH;

      res.json({
        deviceId: id,
        totalWh,
        totalKWh,
        custoEstimado: custoEstimado.toFixed(2),
      });
    } catch (error) {
      res.status(500).json({ error: error.message });
    }
  }

  /**
   * Atualiza um dispositivo existente
   */
  async atualizar(req, res) {
    try {
      const { id } = req.params;
      const { name, description, port } = req.body;

      const device = await prisma.device.update({
        where: { id: Number(id) },
        data: { 
          ...(name && { name }),
          ...(description !== undefined && { description }),
          ...(port && { port })
        }
      });

      res.json(device);
    } catch (error) {
      if (error.code === 'P2025') {
        return res.status(404).json({ error: "Dispositivo não encontrado" });
      }
      res.status(500).json({ error: error.message });
    }
  }

  /**
   * Busca um dispositivo por ID
   */
  async buscarPorId(req, res) {
    try {
      const { id } = req.params;
      const device = await prisma.device.findUnique({
        where: { id: Number(id) },
        include: {
          readings: {
            orderBy: { createdAt: 'desc' },
            take: 10 // Últimas 10 leituras
          }
        }
      });

      if (!device) {
        return res.status(404).json({ error: "Dispositivo não encontrado" });
      }

      res.json(device);
    } catch (error) {
      res.status(500).json({ error: error.message });
    }
  }

  /**
   * Busca um dispositivo por porta/pino
   */
  async buscarPorPorta(req, res) {
    try {
      const { porta } = req.params;
      const device = await prisma.device.findUnique({
        where: { port: String(porta) }
      });

      if (!device) {
        return res.status(404).json({ error: `Nenhum dispositivo cadastrado na porta ${porta}` });
      }

      res.json(device);
    } catch (error) {
      res.status(500).json({ error: error.message });
    }
  }

  /**
   * Associa um nome a uma entrada (pino) do ESP32
   * Se o dispositivo já existir na porta, atualiza o nome
   * Se não existir, cria um novo dispositivo
   */
  async associarNome(req, res) {
    try {
      const { porta, name, description } = req.body;

      if (!porta) {
        return res.status(400).json({ error: "Campo 'porta' é obrigatório" });
      }

      if (!name) {
        return res.status(400).json({ error: "Campo 'name' é obrigatório" });
      }

      // Converte porta para string para garantir consistência
      const portString = String(porta);

      // Tenta encontrar dispositivo existente na porta
      const deviceExistente = await prisma.device.findUnique({
        where: { port: portString }
      });

      let device;

      if (deviceExistente) {
        // Atualiza dispositivo existente
        device = await prisma.device.update({
          where: { id: deviceExistente.id },
          data: {
            name,
            ...(description !== undefined && { description })
          }
        });
      } else {
        // Cria novo dispositivo
        device = await prisma.device.create({
          data: {
            port: portString,
            name,
            ...(description !== undefined && { description })
          }
        });
      }

      res.json({
        message: deviceExistente 
          ? `Nome atualizado para a entrada ${porta}` 
          : `Dispositivo cadastrado na entrada ${porta}`,
        device
      });
    } catch (error) {
      console.error("Erro ao associar nome:", error);
      res.status(500).json({ 
        error: "Erro interno do servidor", 
        details: error.message 
      });
    }
  }
}

