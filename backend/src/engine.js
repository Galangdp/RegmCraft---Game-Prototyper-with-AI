import { spawn } from "child_process";
import { mkdir, readFile, unlink } from "node:fs/promises";
import { GoogleGenAI } from "@google/genai";
import path from "node:path";

const Engine = {
    process: null,

    _ai: {
        ai: null,
        instruction: "",
        schema: null,
        model: ""
    },

    _cwd: "",
    _resolveFn: null,
    _buffer: Buffer.alloc(0),

    /**
     * Menjalankan C Engine sebagai child process untuk berkomunikasi via stdin/stdout.
     * Hanya dipanggi sekali.
     * @param {string} path - Engine binary path
     * @param {string} cwd - Engine binary working directory
     * @param {string} model - Nama model Gemini AI
     * @param {string} apiKey - API Key untuk Gemini AI
     * @returns {Promise<null>} - Resolve kalau berhasil dan reject kalau gagal 
     */
    spawn: function (path, cwd, model, apiKey) {
        return new Promise((resolve, reject) => {
            // Spawn C Engine binary sebagai child process
            Engine.process = spawn(path, [], { cwd: cwd });
            Engine._cwd = cwd;

            Engine._ai.ai = new GoogleGenAI({ apiKey: apiKey });
            Engine._ai.model = model;
            
            // Tangkap output dari C Engine (stdout = response JSON)
            Engine.process.stdout.on("data", (chunk) => {
                Engine._buffer = Buffer.concat([Engine._buffer, chunk]);
    
                while (true) {
                    if (Engine._buffer.length < 4) {
                        return;
                    }
    
                    const messageLength = Engine._buffer.readUInt32LE(0);
                    
                    if (Engine._buffer.length < 4 + messageLength) {
                        return;
                    }
                    
                    const data = Engine._buffer.slice(4, messageLength + 4);
                    
                    Engine._buffer = Engine._buffer.slice(messageLength + 4);
                    Engine._resolveFn(JSON.parse(data.toString()));
                }
            });
    
            // Ketika C Engine selesai
            Engine.process.on("close", (exitCode) => {
                reject(new Error(`[Engine] Process exited with code ${exitCode}.`));
            });
    
            // Handle jika binary tidak ditemukan / tidak bisa di-spawn
            Engine.process.on("error", (err) => {
                reject(new Error(`[Engine] Run C Engine fail: ${err.message}.`));
            });

            Engine.process.on("spawn", async () => {
                try {
                    await Engine._init();
                } catch (error) {
                    reject(error);
                }
                resolve();
            });
        });
    },

    /**
     * Gateway untuk pemanggilan C Engine API.
     * Tidak disarankan pemanggilan langsung, harus via wrapper.
     * @param {"init"|"uinit"|"create"|"edit"|"prompt"} cmd - Nama command
     * @param {Object} [data] - Data optional command
     */
    _send: async function (cmd, data = {}) {
        data.cmd = cmd;
        
        const promise = new Promise((resolve) => {
            Engine._resolveFn = resolve;
        });
        
        const payload = Buffer.from(JSON.stringify(data));
        const head = Buffer.alloc(4);
        
        head.writeUInt32LE(payload.length);
        
        Engine.process.stdin.write(head);
        Engine.process.stdin.write(payload);

        return promise;
    },

    /**
     * Command init.
     * Hanya dipanggil sekali setelah spawn berhasil.
     * @returns {Promise<null>}
     */
    _init: function () {
        return new Promise(async (resolve, reject) => {
            try {
                await mkdir(path.resolve(Engine._cwd, "temp"), { recursive: true });
            } catch (error) {
                reject(new Error("Error create temp directory file."));
            }

            const result = await Engine._send("init");
            
            if (result.status != "ok") {
                return result;
            }
            
            let instructionString = "";
            let schemaString = "";
            let instructionPath = path.resolve(Engine._cwd, result.instruction);
            let schemaPath = path.resolve(Engine._cwd, result.schema);

            try {
                instructionString = await readFile(instructionPath);
                schemaString = await readFile(schemaPath);
            } catch (error) {
                reject(new Error("Error read base configuration file."));
            }
            
            Engine._ai.instruction = instructionString.toString();
            Engine._ai.schema = JSON.parse(schemaString);
    
            await unlink(instructionPath);
            await unlink(schemaPath);

            resolve();
        });
    },

    /**
     * Command saat editor pertama kali dibuka.
     * @returns {Promise<Object>}
     */
    uinit: function () {
        return new Promise(async (resolve) => {
            resolve(await Engine._send("uinit", { paletteId: 0 }));
        });
    },

    /**
     * Command create manual.
     * @param {number} paletteId - Id palette saat ini
     * @param {0 | 1 | 2} type - Entity type yang mau dibuat, 0 untuk person, 1 untuk monster, 2 untuk vehicle
     * @returns {Promise<Object>}
     */
    create: function (paletteId, type) {
        return new Promise(async (resolve) => {
            resolve(await Engine._send("create", { paletteId: paletteId, type: type }));
        });
    },

    /**
     * Command edit.
     * @param {number} paletteId - Id palette saat ini
     * @param {Object} data - Data terbaru entity yang mau di apply
     * @returns {Promise<Object>}
     */
    edit: function (paletteId, data) {
        data.paletteId = paletteId;
        return new Promise(async (resolve) => {
            resolve(await Engine._send("edit", data));
        });
    },

    /**
     * Command create via prompt.
     * @param {number} paletteId - Id palette saat ini
     * @param {string} input - User prompt
     * @returns {Promise<Object>}
     */
    prompt: function (paletteId, input) {
        return new Promise(async (resolve) => {
            // const aiResponse = await Engine._ai.ai.models.generateContent({
            //     model: Engine._ai.model,
            //     systemInstruction: Engine._ai.instruction,
            //     contents: input,
            //     config: {
            //         responseMimeType: "application/json",
            //         responseJsonSchema: Engine._ai.schema
            //     }
            // });
            // console.dir(JSON.parse(aiResponse.text), { depth: null });
    
            const result = await Engine._send("create", { type: 0, paletteId: paletteId });
    
            delete result.status;
            result.type = 0;

            resolve({ status: "ok", items: [result] });
        });
    }
};

export { Engine };
