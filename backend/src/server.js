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

// ─── Setup ───────────────────────────────────────────────────────────────────

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const app = express();
const PORT = process.env.PORT || 3001;

// Path ke binary C Engine (relatif dari root monorepo)
const ENGINE_BINARY = process.env.ENGINE_BINARY_PATH
    ? path.resolve(__dirname, "..", "..", process.env.ENGINE_BINARY_PATH)
    : path.resolve(__dirname, "../../core-engine/build/regm_engine");

// ─── Middleware ───────────────────────────────────────────────────────────────

app.use(cors({ origin: process.env.FRONTEND_URL || "http://localhost:3000" }));
app.use(express.json({ limit: "10mb" }));

// ─── Helper: Invoke C Engine ──────────────────────────────────────────────────

/**
 * Menjalankan C Engine sebagai child process dan berkomunikasi via stdin/stdout.
 * @param {object} payload - JSON payload yang dikirim ke C Engine via stdin
 * @returns {Promise<object>} - JSON response dari C Engine via stdout
 */
function invokeEngine(payload) {
    return new Promise((resolve, reject) => {
        // Spawn C Engine binary sebagai child process
        const engineProcess = spawn(ENGINE_BINARY, [], {
            stdio: ["pipe", "pipe", "pipe"], // [stdin, stdout, stderr]
        });

        let stdoutBuffer = "";
        let stderrBuffer = "";

        // Tangkap output dari C Engine (stdout = response JSON)
        engineProcess.stdout.on("data", (chunk) => {
            stdoutBuffer += chunk.toString();
        });

        // Tangkap error/log dari C Engine (stderr = debug info)
        engineProcess.stderr.on("data", (chunk) => {
            stderrBuffer += chunk.toString();
        });

        // Ketika C Engine selesai
        engineProcess.on("close", (exitCode) => {
            if (exitCode !== 0) {
                console.error(`[Engine] Process exited with code ${exitCode}`);
                console.error(`[Engine] stderr: ${stderrBuffer}`);
                return reject(
                    new Error(
                        `C Engine exited with code ${exitCode}. stderr: ${stderrBuffer}`
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
                        `Gagal parse JSON dari C Engine: ${parseError.message}. Raw stdout: ${stdoutBuffer}`
                    )
                );
            }
        });

        // Handle jika binary tidak ditemukan / tidak bisa di-spawn
        engineProcess.on("error", (err) => {
            reject(new Error(`Gagal menjalankan C Engine: ${err.message}`));
        });

        // Set timeout 30 detik — jika engine tidak merespons, kill paksa
        const timeout = setTimeout(() => {
            engineProcess.kill("SIGKILL");
            reject(new Error("C Engine timeout setelah 30 detik"));
        }, 30_000);

        // Bersihkan timeout jika engine selesai lebih cepat
        engineProcess.on("close", () => clearTimeout(timeout));

        // Kirim request payload ke stdin C Engine lalu tutup stdin
        const jsonPayload = JSON.stringify(payload);
        engineProcess.stdin.write(jsonPayload);
        engineProcess.stdin.end(); // Sinyal ke C Engine bahwa input sudah selesai
    });
}

// ─── Routes ───────────────────────────────────────────────────────────────────

/**
 * Health check — untuk verifikasi backend berjalan
 * GET /health
 */
app.get("/health", (_req, res) => {
    res.json({ status: "ok", engine: ENGINE_BINARY });
});

/**
 * Main endpoint — Generate entity via C Engine
 * POST /api/engine/generate
 *
 * Body (JSON): req_generate_entity schema
 * Response (JSON): res_generate_entity schema
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
