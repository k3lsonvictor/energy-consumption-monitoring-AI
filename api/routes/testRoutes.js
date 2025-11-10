import express from "express";
import { TestController } from "../controllers/TestController.js";

const router = express.Router();
const testController = new TestController();

// Testa o fluxo completo de chat
router.post("/chat", (req, res) => testController.testarChat(req, res));

// Lista dispositivos disponíveis
router.get("/devices", (req, res) => testController.listarDispositivos(req, res));

export default router;

