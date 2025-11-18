import express from "express";
import { PowerController } from "../controllers/PowerController.js";

const router = express.Router();
const powerController = new PowerController();

router.post("/", (req, res) => powerController.criar(req, res));
router.get("/device/:deviceId", (req, res) => powerController.listarPorDevice(req, res));

export default router;

