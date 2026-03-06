"use client";
import { useEffect, useRef, useState, useCallback } from "react";
import renderer from "../../Renderer";
import { useRouter, useParams } from "next/navigation";
import { EngineProvider, useEngine } from "../../lib/engine/EngineContext";
import { useProjects } from "../../hooks/useProjects";
import TilemapManager from "../../lib/engine/TilemapManager";
import TilePalette from "../../components/editor/TilePalette";
import { supabase } from "@/lib/supabase";

function EditorContent() {
    const canvasRef = useRef(null);
    const params = useParams();
    const projectId = params?.id;

    // Project & Scene State
    const { initEngine, character, isLoading: engineLoading } = useEngine();
    const { projects } = useProjects();
    const [status, setStatus] = useState("Initializing...");
    const [syncStatus, setSyncStatus] = useState("idle"); // idle, syncing, saved, error
    const [selectedTileId, setSelectedTileId] = useState(0);
    const [isPainting, setIsPainting] = useState(false);

    // Engine Objects
    const tilemapRef = useRef(new TilemapManager());
    const router = useRouter();

    // ─── Save & Load Logic ────────────────────────────────────────────────────

    const saveProject = useCallback(async () => {
        if (!projectId) return;
        setSyncStatus("syncing");

        const sceneData = {
            tilemap: tilemapRef.current.serialize(),
            character: character ? { id: character.id, name: character.name } : null
        };

        try {
            const { error } = await supabase
                .from("projects")
                .update({ data: sceneData, last_modified: new Date().toISOString() })
                .eq("id", projectId);

            if (error) throw error;
            setSyncStatus("saved");
        } catch (err) {
            console.error("Save error:", err);
            setSyncStatus("error");
        }
    }, [projectId, character]);

    // Cleanup & Debounce Save
    useEffect(() => {
        const timer = setTimeout(() => {
            if (syncStatus === "pending") {
                saveProject();
            }
        }, 2000);
        return () => clearTimeout(timer);
    }, [syncStatus, saveProject]);

    // ─── Initialization ───────────────────────────────────────────────────────

    useEffect(() => {
        if (!canvasRef.current) return;

        const init = async () => {
            try {
                renderer.init(canvasRef.current);
                setStatus("Renderer Ready. Connecting to C-Engine...");
                initEngine();

                // Load project data
                if (projectId) {
                    const { data, error } = await supabase
                        .from("projects")
                        .select("data")
                        .eq("id", projectId)
                        .single();

                    if (data?.data?.tilemap) {
                        tilemapRef.current.deserialize(data.data.tilemap);
                        setStatus("Scene Loaded.");
                    }
                }
            } catch (err) {
                setStatus("Error: " + err.message);
            }
        };

        init();
    }, [initEngine, projectId]);

    // ─── Render Loop ──────────────────────────────────────────────────────────

    useEffect(() => {
        let frameId;
        const loop = () => {
            renderer.begin();

            // Layer 0: Tilemap
            renderer.renderTilemap(tilemapRef.current);

            // Layer 1: Entities (Simulasi)
            if (character) {
                // renderer.drawSprite(...)
            }

            renderer.end('tile_placeholder');
            frameId = requestAnimationFrame(loop);
        };

        loop();
        return () => cancelAnimationFrame(frameId);
    }, [character]);

    // ─── Mouse Handlers ───────────────────────────────────────────────────────

    const handleMouseAction = (e) => {
        if (!canvasRef.current || !isPainting) return;

        const rect = canvasRef.current.getBoundingClientRect();
        const x = e.clientX - rect.left;
        const y = e.clientY - rect.top;

        const tileX = Math.floor(x / tilemapRef.current.tileSize);
        const tileY = Math.floor(y / tilemapRef.current.tileSize);

        if (tilemapRef.current.getTile(tileX, tileY) !== selectedTileId) {
            tilemapRef.current.setTile(tileX, tileY, selectedTileId);
            setSyncStatus("pending"); // Trigger debounce save
        }
    };

    return (
        <div className="flex flex-col h-screen bg-neutral-950 text-white font-sans p-8">
            <header className="flex justify-between items-center mb-8">
                <div>
                    <h1 className="text-2xl font-bold tracking-tight flex items-center gap-3">
                        RegmCraft Editor
                        <span className="text-xs font-normal text-blue-500 bg-blue-500/10 px-2 py-1 rounded">WebGL2 Scene</span>
                    </h1>
                    <div className="flex items-center gap-4 mt-1">
                        <p className="text-neutral-500 text-sm">Status: {status}</p>
                        <div className="flex items-center gap-2">
                            <span className={`w-2 h-2 rounded-full ${syncStatus === 'syncing' ? 'bg-yellow-500 animate-pulse' :
                                    syncStatus === 'saved' ? 'bg-green-500' :
                                        syncStatus === 'error' ? 'bg-red-500' : 'bg-neutral-700'
                                }`} />
                            <span className="text-[10px] uppercase font-bold tracking-widest text-neutral-500">
                                {syncStatus === 'syncing' ? 'Syncing...' :
                                    syncStatus === 'saved' ? 'All changes saved' :
                                        syncStatus === 'error' ? 'Save Error' :
                                            syncStatus === 'pending' ? 'Unsaved changes' : 'Draft'}
                            </span>
                        </div>
                    </div>
                </div>
                <button
                    onClick={() => router.push("/dashboard")}
                    className="text-sm bg-neutral-900 hover:bg-neutral-800 px-4 py-2 rounded-lg border border-neutral-800 transition-colors"
                >
                    Back to Dashboard
                </button>
            </header>

            <div className="flex-1 flex gap-8 overflow-hidden">
                {/* Canvas Area */}
                <div className="flex-1 bg-neutral-900 rounded-3xl border border-neutral-800 overflow-hidden relative group shadow-2xl">
                    <canvas
                        ref={canvasRef}
                        onMouseDown={() => setIsPainting(true)}
                        onMouseUp={() => setIsPainting(false)}
                        onMouseMove={handleMouseAction}
                        className="w-full h-full cursor-crosshair"
                        width={800}
                        height={600}
                    />
                    <div className="absolute bottom-4 left-4 bg-black/50 backdrop-blur-md px-3 py-1 rounded text-xs text-neutral-400 border border-white/5">
                        800 x 600 | 20x20 Tiles
                    </div>
                </div>

                {/* Sidebar */}
                <div className="w-80 flex flex-col gap-6">
                    <TilePalette
                        selectedTileId={selectedTileId}
                        onSelectTile={setSelectedTileId}
                    />

                    <div className="bg-neutral-900 p-6 rounded-3xl border border-neutral-800">
                        <h3 className="font-bold mb-4">Inspector</h3>
                        <div className="space-y-4">
                            <div>
                                <label className="text-xs text-neutral-500 uppercase tracking-wider">Active Character</label>
                                <div className="mt-2 p-3 bg-neutral-950 rounded-xl border border-neutral-800 text-sm font-mono truncate">
                                    {character?.name || "None"}
                                </div>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    );
}

export default function BasicEditorPage() {
    return (
        <EngineProvider>
            <EditorContent />
        </EngineProvider>
    );
}
