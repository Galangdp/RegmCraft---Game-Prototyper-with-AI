# RegmCraft — Game Prototyper with AI 🎮

> Monorepo for the RegmCraft platform: AI-powered 2D game asset prototyping engine.

## Architecture

```
RegmCraft/
├── engine/          ← C rendering engine (JSON I/O via stdin/stdout)
├── backend/         ← NodeJS + Express bridge server
└── frontend/        ← Next.js web application
```

## Core Engine Features
- JSON parser & stringify
- PNG encoder/decoder (zlib)
- PPM decoder (debug)
- Custom material encoder/decoder
- Sprite & Tilemap composition
- Gemini AI integration

## Getting Started

### Frontend
```bash
cd frontend && npm install && npm run dev
```

### Backend Bridge
```bash
cd backend && npm install && npm run dev
```

### Build C Engine (Farel's Core)
Gunakan command ini setiap kali ada update kode C++:
```bash
cd engine
mkdir -p build && cd build
cmake ..
make
```
Binary akan dihasilkan di `engine/build/bin/rcengine`. Backend secara otomatis memanggil path ini.

## Stdin/Stdout Contract
The backend communicates with the C engine via **stdin/stdout JSON**:
- Sends `req_generate_entity.json` payload → C engine stdin
- Receives `res_generate_entity.json` payload ← C engine stdout

See `backend/contracts/` for schema definitions.
