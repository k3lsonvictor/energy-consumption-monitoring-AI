import express from "express";
import cors from "cors";
import { PrismaClient } from "@prisma/client";

const app = express();
const prisma = new PrismaClient();

app.use(cors());
app.use(express.json());

// Listar todos os dispositivos
app.get("/devices", async (req, res) => {
  const devices = await prisma.device.findMany();
  res.json(devices);
});

// Registrar novo dispositivo
app.post("/devices", async (req, res) => {
  const { name, description, port } = req.body;
  const device = await prisma.device.create({ data: { name, description, port } });
  res.json(device);
});

app.post("/readings", async (req, res) => {
    try {
      const { port, energyWh, durationMin } = req.body;
      
      // Log para debug (remover em produção)
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
      const device = await prisma.device.findUnique({ where: { port: String(port) } });
      if (!device) {
        return res.status(404).json({ error: `Nenhum dispositivo cadastrado na porta ${port}` });
      }
    
      // Cria leitura associada ao dispositivo
      const reading = await prisma.reading.create({
        data: {
          deviceId: device.id,
          energyWh: Number(energyWh),
          durationMin: Number(durationMin),
        },
      });
    
      res.json({ message: `Leitura registrada para ${device.name}`, reading });
    } catch (error) {
      console.error("Erro ao processar leitura:", error);
      res.status(500).json({ error: "Erro interno do servidor", details: error.message });
    }
  });

// Consultar todas as leituras de um dispositivo
app.get("/devices/:id/readings", async (req, res) => {
  const { id } = req.params;
  const readings = await prisma.reading.findMany({
    where: { deviceId: Number(id) },
    orderBy: { createdAt: "asc" },
  });
  res.json(readings);
});

// Calcular total de energia e custo estimado
app.get("/devices/:id/summary", async (req, res) => {
  const { id } = req.params;
  const readings = await prisma.reading.findMany({
    where: { deviceId: Number(id) },
  });

  const totalWh = readings.reduce((sum, r) => sum + r.energyWh, 0);
  const totalKWh = totalWh / 1000;
  const custoPorKWh = 0.95; // tarifa média no Brasil
  const custoEstimado = totalKWh * custoPorKWh;

  res.json({
    deviceId: id,
    totalWh,
    totalKWh,
    custoEstimado: custoEstimado.toFixed(2),
  });
});

app.listen(3000, () => console.log("⚡ API rodando em http://localhost:3000"));
