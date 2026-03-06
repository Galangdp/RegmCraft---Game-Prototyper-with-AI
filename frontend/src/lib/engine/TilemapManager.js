/**
 * TilemapManager
 * ==============
 * Mengelola data grid 2D untuk sistem tilemap.
 */
class TilemapManager {
    constructor(width = 20, height = 20, tileSize = 32) {
        this.width = width;
        this.height = height;
        this.tileSize = tileSize;

        // Inisialisasi grid dengan ID 0 (misal: Tanah/Soil)
        this.grid = Array.from({ length: height }, () => Array(width).fill(0));

        // Mapping ID ke tipe/tekstur
        this.tileTypes = {
            0: { id: 0, name: 'Soil', color: [0.5, 0.4, 0.3] },
            1: { id: 1, name: 'Wall', color: [0.4, 0.4, 0.4] },
            2: { id: 2, name: 'Water', color: [0.2, 0.5, 0.8] }
        };
    }

    registerTile(id, name, spriteName = null, color = [0.5, 0.5, 0.5]) {
        this.tileTypes[id] = { id, name, spriteName, color };
    }

    setTile(x, y, tileId) {
        if (x >= 0 && x < this.width && y >= 0 && y < this.height) {
            this.grid[y][x] = tileId;
        }
    }

    getTile(x, y) {
        if (x >= 0 && x < this.width && y >= 0 && y < this.height) {
            return this.grid[y][x];
        }
        return -1;
    }

    serialize() {
        return {
            width: this.width,
            height: this.height,
            grid: this.grid
        };
    }

    deserialize(data) {
        if (!data) return;
        this.width = data.width || 20;
        this.height = data.height || 20;
        this.grid = data.grid || Array.from({ length: this.height }, () => Array(this.width).fill(0));
    }
}

export default TilemapManager;
