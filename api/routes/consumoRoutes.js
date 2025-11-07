import express from "express";
import { ConsumoController } from "../controllers/ConsumoController.js";

const router = express.Router();
const consumoController = new ConsumoController();

router.get("/", (req, res) => consumoController.consultar(req, res));

export default router;

