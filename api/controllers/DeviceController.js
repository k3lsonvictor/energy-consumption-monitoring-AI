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
}

