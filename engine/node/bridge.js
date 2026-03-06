import { spawn } from "child_process";
import { readFile } from "node:fs/promises";
import { GoogleGenAI } from "@google/genai";

const Bridge = {
    ai: null,

    modelName: "gemini-2.5-flash",
    apiKey: "AIzaSyAAFGvHhWS93t24AwAy8l5mI9x6Adr-OfU",
    schema: null,
    instruction: "",

    engine: null,
    resolveFn: null,

    buffer: Buffer.alloc(0),

    init: function () {
        this.engine = spawn("./rcengine");
        this.ai = new GoogleGenAI({ apiKey: this.apiKey });

        this.engine.stdout.on("data", (chunk) => {
            this.buffer = Buffer.concat([this.buffer, chunk]);

            while (true) {
                if (this.buffer.length < 4) {
                    return;
                }

                const messageLength = this.buffer.readUInt32LE(0);
                
                if (this.buffer.length < 4 + messageLength) {
                    return;
                }
                
                const data = this.buffer.slice(4, messageLength + 4);
                
                this.buffer = this.buffer.slice(messageLength + 4);
                this.resolveFn(JSON.parse(data.toString()));
            }
        });
    },

    send: async function (command, data = {}) {
        data.cmd = command;

        const promise = new Promise((resolve, reject) => {
            this.resolveFn = resolve;
        });

        const payload = Buffer.from(JSON.stringify(data));
        const head = Buffer.alloc(4);

        head.writeUInt32LE(payload.length);

        this.engine.stdin.write(head);
        this.engine.stdin.write(payload);

        return promise;
    },

    commandInit: async function () {
        const result = await this.send("init");

        if (result.status != "ok") {
            return result;
        }

        let schemaString = "";

        try {
            this.instruction = await readFile(result.instruction);
            schemaString = await readFile(result.schema);
        } catch (error) {
            return {
                status: "error",
                message: "Error read file init."
            }
        }

        this.schema = JSON.parse(schemaString);
        
        delete result.instruction;
        delete result.schema;
        
        return result;
    },

    commandCreate: async function (paletteId, type) {
        return await Bridge.send("create", { paletteId: paletteId, type: type });
    },

    commandPrompt: async function (input) {
        const aiResponse = await this.ai.models.generateContent({
            model: this.modelName,
            systemInstruction: this.instruction,
            contents: input,
            config: {
                responseMimeType: "application/json",
                responseJsonSchema: this.schema
            }
        });

        console.dir(JSON.parse(aiResponse.text), { depth: null });

        const result = await this.send("create", { type: 0, paletteId: 0 });

        delete result.status;
        return { status: "ok", items: [result] };
    },

    commandEdit: async function (data) {
        return await Bridge.send("edit", data);
    }
};

// Example
async function main() {
    Bridge.init();

    // Command init
    const result = await Bridge.commandInit();
    console.dir(result, { depth: null });

    // extract paletteId for further use
    const paletteId = result.paletteId;

    // Command create
    // Bridge.send("create", { paletteId: paletteId, type: 0 });
    const result2 = await Bridge.commandCreate(paletteId, 0);
    console.dir(result2, { depth: null });

    // Extract new entity data
    const entity0 = {
        paletteId: paletteId,
        modelId: result2.modelId,
        moptionId: result2.moptionId ?? -1,
        parts: result2.parts
    };

    // Edit some entity part
    entity0.parts[0].variantId = 0;
    entity0.parts[0].optionId = 1;

    // Apply edit
    // Bridge.send("edit", entity0);
    const result3 = await Bridge.commandEdit(entity0);
    console.dir(result3, { depth: null });
    
    // Create entities with prompt
    // Bridge.send("prompt", { input: "Create person with weapon knight sword" })
    const result4 = await Bridge.commandPrompt("Create person without weapon");
    console.dir(result4, { depth: null });
}

await main();

/**
 * Format command:
 * 
 * -- Init --
 * -- How To --
 * Bridge.commandInit();
 * -- Response --
 *  {
 *      base: 'temp/base.png',
 *      paletteId: 0,
 *      monsters: [],
 *      vehicles: [],
 *      palettes: [ { name: 'Journey', total: 64 } ],
 *      status: 'ok',
 *      persons: [
 *          {
 *              name: 'Person',
 *              id: 0,
 *              variant: {
 *                  options: [
 *                      { frameId: 0, name: 'Light Skin' },
 *                      { frameId: 1, name: 'Normal Skin' },
 *                      { frameId: 2, name: 'Dark Skin' }
 *                  ],
 *                  name: 'Skin Color'
 *              },
 *              parts: [
 *                  {
 *                      name: 'Weapon',
 *                      variants: [
 *                          { name: 'None', id: -1 },
 *                          {
 *                              options: [
 *                                  { frameId: 4, name: 'Red', id: 0 },
 *                                  { frameId: 5, name: 'Gold', id: 1 },
 *                                  { frameId: 6, name: 'Dark', id: 2 },
 *                                  { frameId: 7, name: 'Wood', id: 3 }
 *                              ],
 *                              frameId: 3,
 *                              name: 'Knight Sword',
 *                              id: 0
 *                          }
 *                      ]
 *                  },
 *                  {
 *                      name: 'Hat',
 *                      variants: [
 *                          { name: 'None', id: -1 },
 *                          {
 *                              options: [
 *                                  { frameId: 9, name: 'Brown', id: 0 }
 *                              ],
 *                              frameId: 8,
 *                              name: 'Cowboy Hat',
 *                              id: 0
 *                          }
 *                      ]
 *                  },
 *                  {
 *                      name: 'Hair',
 *                      variants: [
 *                          { name: 'None', id: -1 },
 *                          {
 *                              options: [
 *                                  { frameId: 11, name: 'Red', id: 0 }
 *                              ],
 *                              frameId: 10,
 *                              name: 'Nerd',
 *                              id: 0
 *                          }
 *                      ]
 *                  },
 *                  {
 *                      name: 'Head',
 *                      variants: [
 *                          {
 *                              frameId: 12,
 *                              name: 'Face',
 *                              id: 0
 *                          }
 *                      ]
 *                  },
 *                  {
 *                      name: 'Leg',
 *                      variants: [
 *                          {
 *                              options: [
 *                                  { frameId: 14, name: 'Brown', id: 0 }
 *                              ],
 *                              frameId: 13,
 *                              name: 'Levis Pant',
 *                              id: 0
 *                          }
 *                      ]
 *                  },
 *                  {
 *                      name: 'Body',
 *                      variants: [
 *                          {
 *                              options: [
 *                                  { frameId: 16, name: 'Brown', id: 0 }
 *                              ],
 *                              frameId: 15,
 *                              name: 'Cowboy Jacket',
 *                              id: 0
 *                          }
 *                      ]
 *                  }
 *              ],
 *          }
 *      ]
 *  }
 * 
 * -- Create Entity --
 * -- How To --
 * Bridge.commandCreate(0, 0 Person | 1 Monster | 2 Vehicle);
 * -- Response --
 *  {
 *      base: 'temp/temp.png',
 *      parts: [
 *          { variantId: -1 },
 *          { variantId: -1 },
 *          { optionId: 0, variantId: 0 },
 *          { optionId: 0, variantId: 0 },
 *          { optionId: 0, variantId: 0 },
 *          { optionId: 0, variantId: 0 }
 *      ],
 *      modelId: 0,
 *      moptionId: 0,
 *      anims: [
 *          {
 *              loop: true,
 *              delays: [ 0.4, 0.1, 0.3 ],
 *              name: 'Idle',
 *              from: 0,
 *              to: 2
 *          },
 *          {
 *              loop: true,
 *              delays: [ 0.1, 0.1, 0.1, 0.1 ],
 *              name: 'Walk',
 *              from: 3,
 *              to: 6
 *          },
 *          {
 *              loop: true,
 *              delays: [ 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1 ],
 *              name: 'Run',
 *              from: 7,
 *              to: 14
 *          },
 *          {
 *              loop: false,
 *              delays: [ 0.1, 0.1, 0.1, 0.1 ],
 *              name: 'Hurt',
 *              from: 15,
 *              to: 18
 *          },
 *          {
 *              loop: false,
 *              delays: [ 0.1, 0.1, 0.1, 0.1, 0.1, 0.1 ],
 *              name: 'Die',
 *              from: 19,
 *              to: 24
 *          }
 *      ],
 *      status: 'ok'
 *  }
 * 
 * -- Edit Entity --
 * -- How To --
 * Bridge.commandEdit({
 *      paletteId: 0, parts: [
 *          { variantId: -1 },
 *          { variantId: -1 },
 *          { optionId: 0, variantId: 0 },
 *          { optionId: 0, variantId: 0 },
 *          { optionId: 0, variantId: 0 },
 *          { optionId: 0, variantId: 0 }
 *      ],
 *      modelId: 0,
 *      moptionId: 0
 * });
 * -- Response --
 * The same as create
 * 
 * -- Create Entity via Prompt --
 * -- How To --
 * Bridge.commandPrompt("Create person with weapon knight sword");
 * -- Response --
 *  {
 *      status: "ok",
 *      items: [
 *          {
 *              base: 'temp/temp.png',
 *              parts: [
 *                  { variantId: -1 },
 *                  { variantId: -1 },
 *                  { optionId: 0, variantId: 0 },
 *                  { optionId: 0, variantId: 0 },
 *                  { optionId: 0, variantId: 0 },
 *                  { optionId: 0, variantId: 0 }
 *              ],
 *              modelId: 0,
 *              moptionId: 0,
 *              anims: [
 *                  {
 *                      loop: true,
 *                      delays: [ 0.4, 0.1, 0.3 ],
 *                      name: 'Idle',
 *                      from: 0,
 *                      to: 2
 *                  },
 *                  {
 *                      loop: true,
 *                      delays: [ 0.1, 0.1, 0.1, 0.1 ],
 *                      name: 'Walk',
 *                      from: 3,
 *                      to: 6
 *                  },
 *                  {
 *                      loop: true,
 *                      delays: [ 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1 ],
 *                      name: 'Run',
 *                      from: 7,
 *                      to: 14
 *                  },
 *                  {
 *                      loop: false,
 *                      delays: [ 0.1, 0.1, 0.1, 0.1 ],
 *                      name: 'Hurt',
 *                      from: 15,
 *                      to: 18
 *                  },
 *                  {
 *                      loop: false,
 *                      delays: [ 0.1, 0.1, 0.1, 0.1, 0.1, 0.1 ],
 *                      name: 'Die',
 *                      from: 19,
 *                      to: 24
 *                  }
 *              ]
 *          }
 *      ]
 *  }
 */
