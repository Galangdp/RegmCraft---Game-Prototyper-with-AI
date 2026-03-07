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

### Init C Engine CMake Config
Gunakan command ini setiap kali perubahan CMake config atau saat init CMake:
- Release Mode:
```bash
cd engine
rm -rf build && mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cd ../..
```
- Debug Mode:
```bash
cd engine
rm -rf build && mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON .. && cp -f compile_commands.json ../compile_commands.json
cd ../..
```

### Build C Engine Executable
Gunakan command ini untuk mem-build C engine menjadi executable setelah CMake sudah di init:
```bash
cd engine/build && cmake --build .
cp -f bin/rcengine ../rcengine && rm bin/rcengine
```
Binary akan dihasilkan di `engine/rcengine`. Backend secara otomatis memanggil path ini.

### Fast Path Build C Engine
Jalankan engine/helper.sh dengan command:
- Untuk init release:
```bash
./engine/helper.sh init-release
```
- Untuk init debug:
```bash
./engine/helper.sh init-debug
```
- Untuk build:
```bash
./engine/helper.sh build
```
- Untuk build tanpa perlu init (langsung build mode release):
```bash
./engine/helper.sh fast-build
```

## Stdin/Stdout Contract
The backend communicates with the C engine via **stdin/stdout JSON**:
- Sends `req_generate_entity.json` payload → C engine stdin
- Receives `res_generate_entity.json` payload ← C engine stdout

See `backend/contracts/` for schema definitions.
