import express from "express";
import { WebhookController } from "../controllers/WebhookController.js";

const router = express.Router();
const webhookController = new WebhookController();

router.post("/chatwoot", (req, res) => webhookController.processar(req, res));

export default router;

