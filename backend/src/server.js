/**
 * RegmCraft Backend Bridge
 * ========================
 * Menjembatani Frontend (Next.js) ↔ C Engine (core-engine/build/regm_engine)
 * Komunikasi dengan C Engine via stdin/stdout menggunakan JSON.
 *
 * Alur: Frontend → POST /api/engine → spawn C Engine → stdin JSON → stdout JSON → response
 */

import express from "express";
import cors from "cors";
import path from "path";
import { fileURLToPath } from "url";
import fs from "fs/promises";
import { config } from "dotenv";
import { Engine } from "./engine.js";

// ─── Setup ───────────────────────────────────────────────────────────────────

const __dirname = path.dirname(fileURLToPath(import.meta.url));

config({ path: path.resolve(__dirname, "../../.env") });

const app = express();
const PORT = process.env.PORT || 3001;
const URL = `${process.env.FRONTEND_URL}:${PORT}`;

// Path ke binary C Engine (relatif dari root monorepo)
// Sekarang poin ke engine/rcengine per Farel's update
const ENGINE_BINARY = path.resolve(__dirname, "../..", process.env.ENGINE_BINARY_PATH, process.env.ENGINE_BINARY_NAME);
const ENGINE_CWD = path.resolve(__dirname, "../..", process.env.ENGINE_BINARY_PATH);
const SHARED_STATE_PATH = path.resolve(ENGINE_CWD, "data/shared_state/supabase_assets.json");

// ─── Middleware ───────────────────────────────────────────────────────────────

app.use(cors({ origin: URL }));
app.use(express.json({ limit: "10mb" }));

// Static folder untuk output C Engine (misal: temp.png)
const OUTPUT_DIR = path.resolve(__dirname, "../..", process.env.ENGINE_BINARY_PATH, process.env.ENGINE_TEMP_PATH);
app.use("/output", express.static(OUTPUT_DIR));

// ─── Helper ──────────────────────────────────────────────────

/**
 * Shared State: Sync Supabase assets to local JSON for C Engine
 */
async function syncSupabaseState(assets) {
    try {
        console.log(`[Bridge] Syncing ${assets.length} assets to local state...`);
        await fs.writeFile(SHARED_STATE_PATH, JSON.stringify(assets, null, 2));
        return true;
    } catch (err) {
        console.error(`[Bridge] Sync Error: ${err.message}`);
        return false;
    }
}

// ─── Routes ───────────────────────────────────────────────────────────────────

/**
 * Engine Initialization
 * POST /api/engine/uinit
 * Dipanggil sekali saat frontend load.
 */
app.post("/api/engine/uinit", async (req, res) => {
    console.log("[API] ← Engine Uinit");
    try {
        // Sync state if assets are provided in the request
        if (req.body.assets) {
            await syncSupabaseState(req.body.assets);
        }
        const response = await Engine.uinit();
        return res.json(response);
    } catch (err) {
        console.error(`[API] ✗ Init Error: ${err.message}`);
        return res.status(500).json({ status: "error", error: err.message });
    }
});

/**
 * Edit Entity
 * POST /api/engine/edit
 */
app.post("/api/engine/edit", async (req, res) => {
    console.log(`[API] ← Engine Edit: paletteId=${req.body.paletteId}`);
    try {
        const response = await Engine.edit(req.body.paletteId, req.body.data);
        return res.json(response);
    } catch (err) {
        console.error(`[API] ✗ Edit Error: ${err.message}`);
        return res.status(500).json({ status: "error", error: err.message });
    }
});

/**
 * Create Entity Manual
 * POST /api/engine/create
 */
app.post("/api/engine/create", async (req, res) => {
    console.log(`[API] ← Engine Create: paletteId=${req.body.paletteId}, type=${req.body.type}`);
    try {
        // Sync state before creation to ensure 'TAP' asset is available
        if (req.body.assets) {
            await syncSupabaseState(req.body.assets);
        }
        const response = await Engine.create(req.body.paletteId, req.body.type);
        return res.json(response);
    } catch (err) {
        console.error(`[API] ✗ Create Error: ${err.message}`);
        return res.status(500).json({ status: "error", error: err.message });
    }
});

/**
 * Create Entity via Prompt
 * POST /api/engine/prompt
 */
app.post("/api/engine/prompt", async (req, res) => {
    console.log(`[API] ← Engine Prompt: paletteId=${req.body.paletteId}, input=${req.body.input}`);
    try {
        // Sync state before creation to ensure 'TAP' asset is available
        if (req.body.assets) {
            await syncSupabaseState(req.body.assets);
        }
        const response = await Engine.prompt(req.body.paletteId, req.body.input);
        return res.json(response);
    } catch (err) {
        console.error(`[API] ✗ Create Error: ${err.message}`);
        return res.status(500).json({ status: "error", error: err.message });
    }
});

/**
 * Ini harusnya ga butuh
 * Clear Temp Files
 * POST /api/engine/clear
 */
app.post("/api/engine/clear", async (req, res) => {
    console.log(`[API] ← Engine Clear: count=${req.body.imagePaths?.length || 0}`);
    try {
        const response = await invokeEngine({ cmd: "clear", ...req.body });
        return res.json(response);
    } catch (err) {
        console.error(`[API] ✗ Clear Error: ${err.message}`);
        return res.status(500).json({ status: "error", error: err.message });
    }
});

/**
 * Health check — untuk verifikasi backend berjalan
 * GET /health
 */
app.get("/health", (_req, res) => {
    res.json({
        status: "ok",
        engine: ENGINE_BINARY,
        platform: process.platform,
        cwd: process.cwd()
    });
});

/**
 * Ini harusnya ga butuh
 * Composition Bridge — Direct rendering dari config frontend ke Mesin C
 * POST /api/engine/compose-and-render
 */
app.post("/api/engine/compose-and-render", async (req, res) => {
    const startTime = Date.now();

    if (!req.body || !req.body.layers) {
        return res.status(400).json({
            status: "error",
            error: "Konfigurasi layer tidak valid",
        });
    }

    console.log(`[Bridge] Incoming composition request (${req.body.layers.length} layers)`);

    try {
        // Paksa action ke generate_entity jika belum ada
        const payload = {
            action: "generate_entity",
            ...req.body
        };

        const engineResponse = await invokeEngine(payload);

        // Antigravity UX: Berikan metadata tambahan dari backend
        const elapsed = Date.now() - startTime;
        if (engineResponse.metadata) {
            engineResponse.metadata.bridge_latency_ms = elapsed;
        }

        return res.json(engineResponse);
    } catch (err) {
        console.error(`[Bridge] ✗ Error: ${err.message}`);
        return res.status(500).json({
            status: "error",
            error: err.message
        });
    }
});

/**
 * Ini harusnya ga butuh
 * Legacy/Generic Engine Endpoint
 * POST /api/engine/generate
 */
app.post("/api/engine/generate", async (req, res) => {
    const startTime = Date.now();

    // Validasi request body
    if (!req.body || !req.body.action) {
        return res.status(400).json({
            status: "error",
            error: 'Request body wajib ada field "action"',
        });
    }

    console.log(
        `[API] ← Request: action="${req.body.action}", entity_type="${req.body.entity_type}"`
    );

    try {
        // Invoke C Engine secara async — server tidak freeze saat rendering
        const engineResponse = await invokeEngine(req.body);

        const elapsed = Date.now() - startTime;
        console.log(`[API] → Response: status="${engineResponse.status}" (${elapsed}ms)`);

        // Tambahkan total latency ke metadata response
        if (engineResponse.metadata) {
            engineResponse.metadata.total_latency_ms = elapsed;
        }

        return res.json(engineResponse);
    } catch (err) {
        console.error(`[API] ✗ Error: ${err.message}`);
        return res.status(500).json({
            status: "error",
            error: err.message,
            request_id: req.body.request_id || null,
        });
    }
});

// ─── Start Server ─────────────────────────────────────────────────────────────

(async () => {
    try {
        await Engine.spawn(ENGINE_BINARY, ENGINE_CWD, process.env.AI_MODEL_NAME, process.env.AI_API_KEY);
    } catch (error) {
        console.error(error.message);
        return;
    }

    app.listen(PORT, () => {
        console.log(`\n🎮 RegmCraft Backend Bridge`);
        console.log(`   Listening on : ${URL}`);
        console.log(`   C Engine     : ${ENGINE_BINARY}`);
        console.log(`   Environment  : ${process.env.NODE_ENV || "development"}\n`);
    });
})();
