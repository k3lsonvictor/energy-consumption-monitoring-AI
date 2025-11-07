import express from "express";
import { ReadingController } from "../controllers/ReadingController.js";

const router = express.Router();
const readingController = new ReadingController();

router.post("/", (req, res) => readingController.criar(req, res));

export default router;

