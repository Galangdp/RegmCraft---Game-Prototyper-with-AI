/**
 * RegmCraft WebGL2 Batching Renderer
 * ==================================
 * Renderer ini didesain untuk efisiensi tinggi (High-Performance).
 *
 * FITUR UTAMA:
 * 1. WebGL2 Batching: Mengurangi Draw Calls dengan mengelompokkan sprite dalam satu buffer.
 * 2. Palette Support: Menggunakan uniform multiplier untuk pewarnaan dinamis.
 * 3. Vertex Buffer Object (VBO): Menyimpan data posisi, texcoord, dan warna secara efisien.
 */

class Renderer {
    constructor() {
        this.gl = null;
        this.program = null;
        this.batchSize = 1000; // Maksimal sprite per batch
        this.verticesPerSprite = 6; // 2 triangle x 3 vertex
        this.floatsPerVertex = 7; // [x, y, u, v, r, g, b]

        // Buffer untuk menyimpan data vertex selama batching
        this.vertexData = new Float32Array(
            this.batchSize * this.verticesPerSprite * this.floatsPerVertex
        );
        this.spriteCount = 0;
        this.vbo = null;

        // Map untuk menyimpan tekstur yang sudah di-load
        this.textures = new Map();
    }

    /**
     * Inisialisasi WebGL2 context dan shaders
     */
    init(canvas) {
        this.gl = canvas.getContext('webgl2', { alpha: true, premultipliedAlpha: false });
        if (!this.gl) throw new Error("WebGL2 not supported");

        const vsSource = `#version 300 es
      layout(location = 0) in vec2 a_position;
      layout(location = 1) in vec2 a_texCoord;
      layout(location = 2) in vec3 a_colorMultiplier;

      uniform vec2 u_resolution;

      out vec2 v_texCoord;
      out vec3 v_colorMultiplier;

      void main() {
        // Konversi dari pixel space ke clip space [-1, 1]
        vec2 zeroToOne = a_position / u_resolution;
        vec2 zeroToTwo = zeroToOne * 2.0;
        vec2 clipSpace = zeroToTwo - 1.0;
        
        gl_Position = vec4(clipSpace * vec2(1, -1), 0, 1);
        v_texCoord = a_texCoord;
        v_colorMultiplier = a_colorMultiplier;
      }
    `;

        const fsSource = `#version 300 es
      precision highp float;

      in vec2 v_texCoord;
      in vec3 v_colorMultiplier;
      uniform sampler2D u_texture;

      out vec4 outColor;

      void main() {
        vec4 texColor = texture(u_texture, v_texCoord);
        // Terapkan color multiplier untuk sistem material/palet
        outColor = vec4(texColor.rgb * v_colorMultiplier, texColor.a);
      }
    `;

        this.program = this._createProgram(vsSource, fsSource);
        this.gl.useProgram(this.program);

        // Setup VBO
        this.vbo = this.gl.createBuffer();
        this.gl.bindBuffer(this.gl.ARRAY_BUFFER, this.vbo);
        this.gl.bufferData(this.gl.ARRAY_BUFFER, this.vertexData.byteLength, this.gl.DYNAMIC_DRAW);

        // Setup Attributes
        const stride = this.floatsPerVertex * Float32Array.BYTES_PER_ELEMENT;

        // a_position (size 2)
        this.gl.enableVertexAttribArray(0);
        this.gl.vertexAttribPointer(0, 2, this.gl.FLOAT, false, stride, 0);

        // a_texCoord (size 2)
        this.gl.enableVertexAttribArray(1);
        this.gl.vertexAttribPointer(1, 2, this.gl.FLOAT, false, stride, 2 * Float32Array.BYTES_PER_ELEMENT);

        // a_colorMultiplier (size 3)
        this.gl.enableVertexAttribArray(2);
        this.gl.vertexAttribPointer(2, 3, this.gl.FLOAT, false, stride, 4 * Float32Array.BYTES_PER_ELEMENT);

        this.gl.enable(this.gl.BLEND);
        this.gl.blendFunc(this.gl.SRC_ALPHA, this.gl.ONE_MINUS_SRC_ALPHA);
    }

    /**
     * Mulai frame baru
     */
    begin() {
        this.gl.clearColor(0, 0, 0, 0);
        this.gl.clear(this.gl.COLOR_BUFFER_BIT);
        this.spriteCount = 0;
    }

    /**
     * Menambahkan sprite ke dalam batch queue
     */
    drawSprite(textureName, x, y, width, height, r = 1.0, g = 1.0, b = 1.0) {
        if (this.spriteCount >= this.batchSize) {
            this.flush(textureName);
        }

        const tex = this.textures.get(textureName);
        if (!tex) return;

        let idx = this.spriteCount * this.verticesPerSprite * this.floatsPerVertex;

        // Triangle 1
        this._addVertex(idx, x, y, 0, 0, r, g, b); idx += 7;
        this._addVertex(idx, x + width, y, 1, 0, r, g, b); idx += 7;
        this._addVertex(idx, x, y + height, 0, 1, r, g, b); idx += 7;

        // Triangle 2
        this._addVertex(idx, x + width, y, 1, 0, r, g, b); idx += 7;
        this._addVertex(idx, x, y + height, 0, 1, r, g, b); idx += 7;
        this._addVertex(idx, x + width, y + height, 1, 1, r, g, b);

        this.spriteCount++;
    }

    _addVertex(idx, x, y, u, v, r, g, b) {
        this.vertexData[idx + 0] = x;
        this.vertexData[idx + 1] = y;
        this.vertexData[idx + 2] = u;
        this.vertexData[idx + 3] = v;
        this.vertexData[idx + 4] = r;
        this.vertexData[idx + 5] = g;
        this.vertexData[idx + 6] = b;
    }

    /**
     * Kirim data buffer ke GPU dan lakukan Draw Call (Batching)
     */
    flush(textureName) {
        if (this.spriteCount === 0) return;

        const gl = this.gl;
        const tex = this.textures.get(textureName);

        gl.bindTexture(gl.TEXTURE_2D, tex);

        // Set resolution uniform
        const resLoc = gl.getUniformLocation(this.program, "u_resolution");
        gl.uniform2f(resLoc, gl.canvas.width, gl.canvas.height);

        // Update buffer data
        gl.bindBuffer(gl.ARRAY_BUFFER, this.vbo);
        gl.bufferSubData(gl.ARRAY_BUFFER, 0, this.vertexData.subarray(0, this.spriteCount * this.verticesPerSprite * this.floatsPerVertex));

        // Draw
        gl.drawArrays(gl.TRIANGLES, 0, this.spriteCount * this.verticesPerSprite);

        this.spriteCount = 0;
    }

    /**
     * End frame
     */
    end(textureName) {
        this.flush(textureName);
    }

    // --- Internals ---

    _createProgram(vsSource, fsSource) {
        const gl = this.gl;
        const vs = this._loadShader(gl.VERTEX_SHADER, vsSource);
        const fs = this._loadShader(gl.FRAGMENT_SHADER, fsSource);
        const program = gl.createProgram();
        gl.attachShader(program, vs);
        gl.attachShader(program, fs);
        gl.linkProgram(program);
        if (!gl.getProgramParameter(program, gl.LINK_STATUS)) {
            throw new Error(gl.getProgramInfoLog(program));
        }
        return program;
    }

    _loadShader(type, source) {
        const gl = this.gl;
        const shader = gl.createShader(type);
        gl.shaderSource(shader, source);
        gl.compileShader(shader);
        if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
            const msg = gl.getShaderInfoLog(shader);
            gl.deleteShader(shader);
            throw new Error(msg);
        }
        return shader;
    }

    async loadTexture(name, url) {
        return new Promise((resolve) => {
            const img = new Image();
            img.onload = () => {
                const gl = this.gl;
                const tex = gl.createTexture();
                gl.bindTexture(gl.TEXTURE_2D, tex);
                gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, img);

                // Pixel art styling: Nearest filtering
                gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
                gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
                gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
                gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);

                this.textures.set(name, tex);
                resolve(tex);
            };
            img.src = url;
        });
    }
}

export default new Renderer();
