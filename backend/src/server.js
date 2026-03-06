/**
 * RegmCraft Backend Bridge
 * ========================
 * Menjembatani Frontend (Next.js) ↔ C Engine (core-engine/build/regm_engine)
 * Komunikasi dengan C Engine via stdin/stdout menggunakan JSON.
 *
 * Alur: Frontend → POST /api/engine → spawn C Engine → stdin JSON → stdout JSON → response
 */

import "dotenv/config";
import express from "express";
import cors from "cors";
import { spawn } from "child_process";
import path from "path";
import { fileURLToPath } from "url";
import fs from "fs/promises";

// ─── Setup ───────────────────────────────────────────────────────────────────

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const app = express();
const PORT = process.env.PORT || 3001;

// Path ke binary C Engine (relatif dari root monorepo)
// Sekarang poin ke engine/build/bin/rcengine per Farel's update
const ENGINE_BINARY = process.env.ENGINE_BINARY_PATH
    ? path.resolve(__dirname, "..", "..", process.env.ENGINE_BINARY_PATH)
    : path.resolve(__dirname, "../../engine/build/bin/rcengine");

const ENGINE_CWD = path.resolve(__dirname, "../../engine");
const SHARED_STATE_PATH = path.resolve(ENGINE_CWD, "data/shared_state/supabase_assets.json");

// ─── Middleware ───────────────────────────────────────────────────────────────

app.use(cors({ origin: process.env.FRONTEND_URL || "http://localhost:3000" }));
app.use(express.json({ limit: "10mb" }));

// Static folder untuk output C Engine (misal: temp.png)
const OUTPUT_DIR = path.resolve(__dirname, "../../core-engine/output");
app.use("/output", express.static(OUTPUT_DIR));

// ─── Helper: Invoke C Engine ──────────────────────────────────────────────────

// ─── Helper: Invoke C Engine ──────────────────────────────────────────────────

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

/**
 * Menjalankan C Engine sebagai child process dan berkomunikasi via stdin/stdout.
 * @param {object} payload - JSON payload yang dikirim ke C Engine via stdin
 * @returns {Promise<object>} - JSON response dari C Engine via stdout
 */
function invokeEngine(payload) {
    return new Promise((resolve, reject) => {
        console.log(`[Engine] Spawning: ${ENGINE_BINARY}`);
        console.log(`[Engine] CWD: ${ENGINE_CWD}`);

        // Spawn C Engine binary sebagai child process
        const engineProcess = spawn(ENGINE_BINARY, [], {
            stdio: ["pipe", "pipe", "pipe"], // [stdin, stdout, stderr]
            cwd: ENGINE_CWD // Pastikan engine bisa baca data/matroot.json
        });

        let stdoutBuffer = "";
        let stderrBuffer = "";

        // Tangkap output dari C Engine (stdout = response JSON)
        engineProcess.stdout.on("data", (chunk) => {
            stdoutBuffer += chunk.toString();
        });

        // Tangkap error/log dari C Engine (stderr = debug info dari Farel)
        engineProcess.stderr.on("data", (chunk) => {
            const msg = chunk.toString();
            stderrBuffer += msg;
            // Langsung log stderr ke console backend agar Senior Engineer bisa liat debug log C secara real-time
            process.stderr.write(`[C-LOG] ${msg}`);
        });

        // Ketika C Engine selesai
        engineProcess.on("close", (exitCode) => {
            if (exitCode !== 0) {
                console.error(`[Engine] Process exited with code ${exitCode}`);
                return reject(
                    new Error(
                        `C Engine exited with code ${exitCode}. stderr: ${stderrBuffer.trim()}`
                    )
                );
            }

            // Parse JSON dari stdout C Engine
            try {
                const result = JSON.parse(stdoutBuffer.trim());
                resolve(result);
            } catch (parseError) {
                reject(
                    new Error(
                        `Gagal parse JSON dari C Engine: ${parseError.message}. Raw stdout: ${stdoutBuffer.substring(0, 100)}...`
                    )
                );
            }
        });

        // Handle jika binary tidak ditemukan / tidak bisa di-spawn
        engineProcess.on("error", (err) => {
            reject(new Error(`Gagal menjalankan C Engine: ${err.message}`));
        });

        // Set timeout 30 detik — jika engine tidak merespons, kill paksa (Hukum Antigravity)
        const timeout = setTimeout(() => {
            engineProcess.kill("SIGKILL");
            reject(new Error("C Engine timeout setelah 30 detik"));
        }, 30_000);

        // Bersihkan timeout jika engine selesai lebih cepat
        engineProcess.once("close", () => clearTimeout(timeout));

        // Kirim request payload ke stdin C Engine lalu tutup stdin
        try {
            const jsonPayload = JSON.stringify(payload);
            engineProcess.stdin.write(jsonPayload, (err) => {
                if (err) {
                    console.error(`[Engine] Stdin write error: ${err.message}`);
                    engineProcess.kill();
                    reject(err);
                } else {
                    engineProcess.stdin.end(); // Akhiri stream stdin
                }
            });
        } catch (stringifyError) {
            engineProcess.kill();
            reject(new Error(`Payload invalid: ${stringifyError.message}`));
        }
    });
}

// ─── Routes ───────────────────────────────────────────────────────────────────

/**
 * Engine Initialization
 * POST /api/engine/init
 * Dipanggil sekali saat frontend load.
 */
app.post("/api/engine/init", async (req, res) => {
    console.log("[API] ← Engine Init");
    try {
        // Sync state if assets are provided in the request
        if (req.body.assets) {
            await syncSupabaseState(req.body.assets);
        }
        const response = await invokeEngine({ cmd: "init" });
        return res.json(response);
    } catch (err) {
        console.error(`[API] ✗ Init Error: ${err.message}`);
        return res.status(500).json({ status: "error", error: err.message });
    }
});

/**
 * Edit Entity (Character/Monster/Vehicle)
 * POST /api/engine/edit
 */
app.post("/api/engine/edit", async (req, res) => {
    console.log(`[API] ← Engine Edit: type=${req.body.type}`);
    try {
        const response = await invokeEngine({ cmd: "edit", ...req.body });
        return res.json(response);
    } catch (err) {
        console.error(`[API] ✗ Edit Error: ${err.message}`);
        return res.status(500).json({ status: "error", error: err.message });
    }
});

/**
 * Create Entity (Empty or via Prompt)
 * POST /api/engine/create
 */
app.post("/api/engine/create", async (req, res) => {
    console.log(`[API] ← Engine Create: prompt=${req.body.prompt || "empty"}`);
    try {
        // Sync state before creation to ensure 'TAP' asset is available
        if (req.body.assets) {
            await syncSupabaseState(req.body.assets);
        }
        // Gunakan payload apa adanya (mendukung cmd: create atau create-empty)
        const payload = req.body.cmd ? req.body : { cmd: "create", ...req.body };
        const response = await invokeEngine(payload);
        return res.json(response);
    } catch (err) {
        console.error(`[API] ✗ Create Error: ${err.message}`);
        return res.status(500).json({ status: "error", error: err.message });
    }
});

/**
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

app.listen(PORT, () => {
    console.log(`\n🎮 RegmCraft Backend Bridge`);
    console.log(`   Listening on : http://localhost:${PORT}`);
    console.log(`   C Engine     : ${ENGINE_BINARY}`);
    console.log(`   Environment  : ${process.env.NODE_ENV || "development"}\n`);
});
