import express from "express";
import { DeviceController } from "../controllers/DeviceController.js";

const router = express.Router();
const deviceController = new DeviceController();

router.get("/", (req, res) => deviceController.listar(req, res));
router.post("/", (req, res) => deviceController.criar(req, res));
router.get("/:id/readings", (req, res) => deviceController.listarLeituras(req, res));
router.get("/:id/summary", (req, res) => deviceController.resumo(req, res));

export default router;

