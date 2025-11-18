import express from "express";
import cors from "cors";

// Rotas
import deviceRoutes from "./routes/deviceRoutes.js";
import readingRoutes from "./routes/readingRoutes.js";
import powerRoutes from "./routes/powerRoutes.js";
import webhookRoutes from "./routes/webhookRoutes.js";
import consumoRoutes from "./routes/consumoRoutes.js";
import testRoutes from "./routes/testRoutes.js";

const app = express();

// Middlewares
app.use(cors());
app.use(express.json());

// Rotas
app.use("/devices", deviceRoutes);
app.use("/readings", readingRoutes);
app.use("/power", powerRoutes);
app.use("/webhook", webhookRoutes);
app.use("/consumo", consumoRoutes);
app.use("/test", testRoutes);

// Health check
app.get("/health", (req, res) => {
  res.json({ status: "ok", timestamp: new Date().toISOString() });
});

const PORT = process.env.PORT || 5000;
app.listen(PORT, () => console.log(`⚡ API rodando em http://localhost:${PORT}`));
