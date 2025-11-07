import { PrismaClient } from "@prisma/client";
import { CUSTO_POR_KWH } from "../../config/constants.js";

const prisma = new PrismaClient();

/**
 * Serviço para buscar e processar dados de consumo
 */
export class ConsumoService {
  /**
   * Busca dados de consumo filtrados por dispositivo e período
   * @param {number|null} deviceId - ID do dispositivo (null para todos)
   * @param {string} periodo - Período: "hoje", "semana", "mes", "total"
   * @returns {Promise<object>} Dados de consumo formatados
   */
  async buscarDadosConsumo(deviceId = null, periodo = "total") {
    try {
      let whereClause = {};
      
      if (deviceId) {
        whereClause.deviceId = Number(deviceId);
      }

      // Filtro por período
      whereClause = this._aplicarFiltroPeriodo(whereClause, periodo);

      const readings = await prisma.reading.findMany({
        where: whereClause,
        include: { device: true },
        orderBy: { createdAt: "desc" },
      });

      const devices = deviceId 
        ? [await prisma.device.findUnique({ where: { id: Number(deviceId) } })]
        : await prisma.device.findMany();

      const dados = devices.map(device => {
        const deviceReadings = readings.filter(r => r.deviceId === device.id);
        return this._calcularEstatisticasDispositivo(device, deviceReadings);
      });

      return {
        periodo,
        resumo: this._calcularResumoGeral(dados),
        dispositivos: dados,
      };
    } catch (error) {
      console.error("Erro ao buscar dados de consumo:", error);
      throw error;
    }
  }

  /**
   * Aplica filtro de período ao whereClause
   * @private
   */
  _aplicarFiltroPeriodo(whereClause, periodo) {
    const novoWhere = { ...whereClause };

    if (periodo === "hoje") {
      const hoje = new Date();
      hoje.setHours(0, 0, 0, 0);
      novoWhere.createdAt = { ...novoWhere.createdAt, gte: hoje };
    } else if (periodo === "semana") {
      const semanaAtras = new Date();
      semanaAtras.setDate(semanaAtras.getDate() - 7);
      novoWhere.createdAt = { ...novoWhere.createdAt, gte: semanaAtras };
    } else if (periodo === "mes") {
      const mesAtras = new Date();
      mesAtras.setMonth(mesAtras.getMonth() - 1);
      novoWhere.createdAt = { ...novoWhere.createdAt, gte: mesAtras };
    }
    // "total" não adiciona filtro de data

    return novoWhere;
  }

  /**
   * Calcula estatísticas de um dispositivo
   * @private
   */
  _calcularEstatisticasDispositivo(device, readings) {
    const totalWh = readings.reduce((sum, r) => sum + r.energyWh, 0);
    const totalKWh = totalWh / 1000;
    const custoEstimado = totalKWh * CUSTO_POR_KWH;
    const totalMinutos = readings.reduce((sum, r) => sum + r.durationMin, 0);

    return {
      dispositivo: device.name,
      porta: device.port,
      totalWh: totalWh.toFixed(2),
      totalKWh: totalKWh.toFixed(2),
      custoEstimado: custoEstimado.toFixed(2),
      quantidadeLeituras: readings.length,
      tempoTotalMinutos: totalMinutos,
      ultimaLeitura: readings[0]?.createdAt || null,
    };
  }

  /**
   * Calcula resumo geral de todos os dispositivos
   * @private
   */
  _calcularResumoGeral(dados) {
    return dados.reduce((acc, d) => ({
      totalWh: (parseFloat(acc.totalWh || 0) + parseFloat(d.totalWh)).toFixed(2),
      totalKWh: (parseFloat(acc.totalKWh || 0) + parseFloat(d.totalKWh)).toFixed(2),
      custoTotal: (parseFloat(acc.custoTotal || 0) + parseFloat(d.custoEstimado)).toFixed(2),
    }), {});
  }
}

