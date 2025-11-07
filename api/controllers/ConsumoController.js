import { ConsumoService } from "../services/consumo/ConsumoService.js";

/**
 * Controlador para consultas de consumo
 */
export class ConsumoController {
  constructor() {
    this.consumoService = new ConsumoService();
  }

  /**
   * Consulta dados de consumo
   */
  async consultar(req, res) {
    try {
      const { deviceId, periodo } = req.query;
      const dados = await this.consumoService.buscarDadosConsumo(
        deviceId, 
        periodo || "total"
      );
      res.json(dados);
    } catch (error) {
      res.status(500).json({ error: error.message });
    }
  }
}

