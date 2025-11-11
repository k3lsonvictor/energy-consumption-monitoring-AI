import express from "express";
import { DeviceController } from "../controllers/DeviceController.js";

const router = express.Router();
const deviceController = new DeviceController();

// Rotas específicas devem vir antes das rotas com parâmetros
router.get("/", (req, res) => deviceController.listar(req, res));
router.post("/", (req, res) => deviceController.criar(req, res));
router.post("/associar", (req, res) => deviceController.associarNome(req, res));
router.get("/porta/:porta", (req, res) => deviceController.buscarPorPorta(req, res));
router.get("/:id/readings", (req, res) => deviceController.listarLeituras(req, res));
router.get("/:id/summary", (req, res) => deviceController.resumo(req, res));
router.get("/:id", (req, res) => deviceController.buscarPorId(req, res));
router.put("/:id", (req, res) => deviceController.atualizar(req, res));

export default router;

