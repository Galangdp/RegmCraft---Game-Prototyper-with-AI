"use client";
import { useState, useEffect, useMemo, useRef, useCallback } from "react";
import {
    Box,
    Layers,
    Settings,
    Play,
    Save,
    ChevronLeft,
    Search,
    Database,
    Image as ImageIcon,
    MousePointer2,
    Brush,
    Eraser,
    Type,
    Circle,
    Square,
    Zap,
    Plus,
    Trash2,
    Trash
} from "lucide-react";
import { motion, AnimatePresence } from "framer-motion";
import { useParams, useRouter } from "next/navigation";
import { supabase } from "@/lib/supabase";
import renderer from "../../../Renderer";
import TilemapManager from "../../../lib/engine/TilemapManager";
import TilePalette from "../../../components/editor/TilePalette";
import AssetCreationModal from "../../../components/editor/AssetCreationModal";

export default function EditorPage() {
    const params = useParams();
    const router = useRouter();
    const canvasRef = useRef(null);
    const tilemapRef = useRef(new TilemapManager());

    const [projectId, setProjectId] = useState(null);
    const [projectName, setProjectName] = useState("Loading...");
    const [activeSidebar, setActiveSidebar] = useState("assets");
    const [assetCategory, setAssetCategory] = useState("entities"); // entities, environment
    const [engineStatus, setEngineStatus] = useState("idle");
    const [syncStatus, setSyncStatus] = useState("idle"); // idle, syncing, saved, error, pending
    const [selectedTileId, setSelectedTileId] = useState(0);
    const [activeTool, setActiveTool] = useState("select"); // select, brush, eraser, character, monster, tree, rock, bush, bridge
    const [aiPrompt, setAiPrompt] = useState("");

    const [isAssetModalOpen, setIsAssetModalOpen] = useState(false);
    const [registryAssets, setRegistryAssets] = useState([]);

    // Scene State
    const [entities, setEntities] = useState([]);
    const [selectedEntityId, setSelectedEntityId] = useState(null);
    const [selectedTile, setSelectedTile] = useState(null); // {x, y, id, name}

    const selectedEntity = useMemo(() =>
        entities.find(e => e.id === selectedEntityId),
        [entities, selectedEntityId]
    );

    // ─── Project Loading ──────────────────────────────────────────────────────

    const fetchRegistryAssets = async () => {
        try {
            const { data, error } = await supabase
                .from('assets_registry')
                .select('*')
                .order('created_at', { ascending: false });

            if (error) throw error;
            const assets = data || [];
            setRegistryAssets(assets);

            // Parallel loading of all textures to GPU
            const loadPromises = assets.map(async (asset, index) => {
                await renderer.loadTexture(asset.name, asset.image_url);

                // If it's a tile asset, register in TilemapManager
                if (asset.type?.toLowerCase().includes('tile')) {
                    tilemapRef.current.registerTile(10 + index, asset.name, asset.name);
                }
            });

            await Promise.all(loadPromises);
            console.log(`[Editor] All ${assets.length} registry assets loaded.`);
        } catch (err) {
            console.error("Fetch registry error:", err);
        }
    };

    const handleUpdateRegistry = async (entity) => {
        if (!entity?.image_url) return;

        setSyncStatus("syncing");
        try {
            // Find by image_url as it's the most reliable link to the registry
            const asset = registryAssets.find(a => a.image_url === entity.image_url);
            if (!asset) {
                console.error("Asset not found in registry for URL:", entity.image_url);
                setSyncStatus("error");
                return;
            }

            const { error } = await supabase
                .from('assets_registry')
                .update({ name: entity.name })
                .eq('id', asset.id);

            if (error) throw error;

            await fetchRegistryAssets();
            setSyncStatus("saved");
            alert(`Registry for "${entity.name}" updated successfully!`);
        } catch (err) {
            console.error("Update registry error:", err);
            setSyncStatus("error");
            alert("Failed to update registry: " + err.message);
        }
    };

    useEffect(() => {
        if (!params?.id) return;
        setProjectId(params.id);

        const loadProject = async () => {
            try {
                const { data, error } = await supabase
                    .from("projects")
                    .select("*")
                    .eq("id", params.id)
                    .single();

                if (error) throw error;

                setProjectName(data.name || "Untitled Project");

                // Cek data (column name in schema is 'data')
                const projectData = data.data || {};

                if (projectData.tilemap) {
                    tilemapRef.current.deserialize(projectData.tilemap);
                }
                if (projectData.entities || projectData.objects) {
                    const loadedEntities = projectData.entities || projectData.objects;
                    // Ensure each entity has a type for sprite rendering
                    const prioritizedEntities = loadedEntities.map(ent => ({
                        ...ent,
                        type: ent.type || (ent.name.toLowerCase().includes('mob') ? 'monster' : 'character')
                    }));

                    // Load textures for custom assets in entities
                    prioritizedEntities.forEach(ent => {
                        if (ent.image_url) {
                            renderer.loadTexture(ent.name, ent.image_url);
                        }
                    });

                    setEntities(prioritizedEntities);
                    if (prioritizedEntities.length > 0) {
                        setSelectedEntityId(prioritizedEntities[0].id);
                    }
                }
                setSyncStatus("saved");
            } catch (err) {
                console.error("Load error:", err);
                setSyncStatus("error");
            }
        };

        loadProject();
        fetchRegistryAssets();
    }, [params]);

    // ─── WebGL Initialization ────────────────────────────────────────────────
    useEffect(() => {
        if (!canvasRef.current) return;
        try {
            renderer.init(canvasRef.current);
            // Default offsets agar tidak undefined saat awal
            renderer.offsetX = 0;
            renderer.offsetY = 0;
            setEngineStatus("connected");
        } catch (err) {
            console.error("WebGL Init Error:", err);
            setEngineStatus("error");
        }
    }, []); // Run once on mount

    // ─── WebGL Render Loop ───────────────────────────────────────────────────
    useEffect(() => {
        let frameId;
        const loop = () => {
            if (!renderer || !renderer.gl) {
                frameId = requestAnimationFrame(loop);
                return;
            }
            renderer.begin();

            // Layer 0: Tilemap (Update offsets internal)
            renderer.renderTilemap(tilemapRef.current, selectedTile);

            // Layer 1: Entities (Image Sprites)
            entities.forEach(ent => {
                const drawX = (renderer.offsetX || 0) + ent.x;
                const drawY = (renderer.offsetY || 0) + ent.y;

                // Use sprite rendering instead of color boxes
                const spriteName = ent.image_url ? ent.name : (ent.type || 'character');

                // Draw selection highlight for entities
                if (selectedEntityId === ent.id) {
                    // Draw a yellow glow/border using white pixels
                    renderer._drawTileInternal(drawX - 18, drawY - 18, 36, 36, 1.0, 0.8, 0.0);
                }

                renderer.drawSprite(spriteName, drawX - 16, drawY - 16, 32, 32);
            });

            renderer.end('tile_placeholder');
            frameId = requestAnimationFrame(loop);
        };

        loop();
        return () => cancelAnimationFrame(frameId);
    }, [entities, selectedEntityId, selectedTile]);

    const isReady = engineStatus === "connected" && syncStatus !== "error";

    // ─── Auto-Save Persistence ────────────────────────────────────────────────

    const saveProject = useCallback(async () => {
        if (!projectId) return;
        setSyncStatus("syncing");

        const projectData = {
            tilemap: tilemapRef.current.serialize(),
            entities: entities
        };

        try {
            const { error } = await supabase
                .from("projects")
                .update({ data: projectData, last_modified: new Date().toISOString() })
                .eq("id", projectId);

            if (error) throw error;
            setSyncStatus("saved");
        } catch (err) {
            console.error("Save error:", err);
            setSyncStatus("error");
        }
    }, [projectId, entities]);

    // Debounce timer
    useEffect(() => {
        if (syncStatus !== "pending") return;

        const timer = setTimeout(() => {
            saveProject();
        }, 2000);

        return () => clearTimeout(timer);
    }, [syncStatus, saveProject]);

    // ─── AI Sync Bridge ────────────────────────────────────────────────────────

    const callEngineBridge = async (action, data = {}) => {
        setEngineStatus("processing");
        const payload = {
            action,
            data: { ...data, projectId },
            timestamp: new Date().toISOString()
        };

        try {
            const response = await fetch("/api/engine", {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify(payload)
            });

            const result = await response.json();

            if (action === "GENERATE_SCENE" && result.output?.entities) {
                const newEntities = result.output.entities.map(e => ({
                    id: `ai-${Date.now()}-${Math.random()}`,
                    ...e,
                    type: e.type || (e.name.toLowerCase().includes('mob') ? 'monster' : 'character')
                }));
                setEntities(prev => [...prev, ...newEntities]);
                setSyncStatus("pending");
                setActiveSidebar("layers"); // Jump to hierarchy to show new items
            }

            setEngineStatus("connected");
            return result;
        } catch (error) {
            console.error("[Engine Bridge] Error:", error);
            setEngineStatus("error");
            return null;
        }
    };

    // ─── Interaction Handlers ──────────────────────────────────────────────────

    const handleCanvasInteraction = (e) => {
        if (!canvasRef.current) return;

        const rect = canvasRef.current.getBoundingClientRect();
        // Scale mouse coordinates to match canvas internal resolution (800x600)
        const mouseX = (e.clientX - rect.left) * (canvasRef.current.width / rect.width);
        const mouseY = (e.clientY - rect.top) * (canvasRef.current.height / rect.height);

        // Alignment Fix: World Space coordinate centered on grid
        const worldX = mouseX - (renderer.offsetX || 0);
        const worldY = mouseY - (renderer.offsetY || 0);

        if (activeTool === "brush" && e.buttons === 1) {
            const tileX = Math.floor(worldX / tilemapRef.current.tileSize);
            const tileY = Math.floor(worldY / tilemapRef.current.tileSize);

            if (tilemapRef.current.getTile(tileX, tileY) !== selectedTileId) {
                tilemapRef.current.setTile(tileX, tileY, selectedTileId);
                setSyncStatus("pending");
            }
        } else if (activeTool === "select" && e.type === "mousedown") {
            // 1. Check Entities
            const closestEntity = entities.find(ent => {
                const dist = Math.sqrt((ent.x - worldX) ** 2 + (ent.y - worldY) ** 2);
                return dist < 20;
            });

            if (closestEntity) {
                setSelectedEntityId(closestEntity.id);
                setSelectedTile(null);
            } else {
                // 2. Check Tiles
                const tileX = Math.floor(worldX / tilemapRef.current.tileSize);
                const tileY = Math.floor(worldY / tilemapRef.current.tileSize);
                const tileId = tilemapRef.current.getTile(tileX, tileY);

                if (tileId > 0) {
                    const type = tilemapRef.current.tileTypes[tileId];
                    setSelectedTile({ x: tileX, y: tileY, id: tileId, name: type?.name || 'Tile' });
                    setSelectedEntityId(null);
                } else {
                    setSelectedEntityId(null);
                    setSelectedTile(null);
                }
            }
        } else if (activeTool === "eraser" && (e.buttons === 1 || e.type === "mousedown")) {
            // Restore Eraser Logic
            let changed = false;

            // 1. Reset Tile to Soil (0)
            const tileX = Math.floor(worldX / tilemapRef.current.tileSize);
            const tileY = Math.floor(worldY / tilemapRef.current.tileSize);
            if (tilemapRef.current.getTile(tileX, tileY) > 0) {
                tilemapRef.current.setTile(tileX, tileY, 0);
                changed = true;
            }

            // 2. Erase Entity
            const closestEntity = entities.find(ent => {
                const dist = Math.sqrt((ent.x - worldX) ** 2 + (ent.y - worldY) ** 2);
                return dist < 20;
            });
            if (closestEntity) {
                setEntities(prev => prev.filter(o => o.id !== closestEntity.id));
                changed = true;
            }

            if (changed) setSyncStatus("pending");

        } else if ((["character", "monster", "tree", "rock", "bush", "bridge"].includes(activeTool) || registryAssets.some(a => a.name === activeTool)) && e.type === "mousedown") {
            const assetFromRegistry = registryAssets.find(a => a.name === activeTool);

            const newEnt = {
                id: `ent-${Date.now()}-${Math.random()}`,
                type: assetFromRegistry ? assetFromRegistry.type : activeTool,
                name: assetFromRegistry ? assetFromRegistry.name : `${activeTool.charAt(0).toUpperCase() + activeTool.slice(1)}_${entities.length + 1}`,
                image_url: assetFromRegistry?.image_url || null,
                x: worldX,
                y: worldY,
                colorR: 1.0, colorG: 1.0, colorB: 1.0 // Unused for image sprites but kept for schema
            };
            setEntities(prev => [...prev, newEnt]);
            setSelectedEntityId(newEnt.id);
            setSelectedTile(null);
            setSyncStatus("pending");
        }
    };

    const handleQuickSpawn = (type) => {
        setActiveTool(type);
    };

    return (
        <div className="flex h-screen bg-neutral-950 text-white overflow-hidden font-sans">
            {/* 1. LEFT SIDEBAR */}
            <aside className="w-72 border-r border-neutral-900 flex flex-col bg-neutral-950/50 backdrop-blur-xl">
                <header className="h-14 border-b border-neutral-900 flex items-center px-4 space-x-3">
                    <button onClick={() => router.push("/dashboard")} className="p-1 hover:bg-neutral-800 rounded-md text-neutral-500 transition-colors">
                        <ChevronLeft size={18} />
                    </button>
                    <span className="text-sm font-semibold truncate text-neutral-300">{projectName}</span>
                </header>

                <nav className="flex items-center border-b border-neutral-900 px-2 bg-neutral-900/20">
                    <TabButton active={activeSidebar === "assets"} onClick={() => setActiveSidebar("assets")} icon={Box} label="Assets" />
                    <TabButton active={activeSidebar === "layers"} onClick={() => setActiveSidebar("layers")} icon={Layers} label="Hierarchy" />
                </nav>

                <div className="flex-1 overflow-y-auto p-4 custom-scrollbar">
                    {activeSidebar === 'assets' ? (
                        <div className="space-y-6">
                            <div className="flex bg-neutral-900 p-1 rounded-xl border border-neutral-800 mb-2 items-center">
                                <button
                                    onClick={() => setAssetCategory("entities")}
                                    className={`flex-1 py-1.5 text-[10px] font-bold uppercase tracking-wider rounded-lg transition-all ${assetCategory === "entities" ? "bg-blue-600 text-white shadow-lg" : "text-neutral-500 hover:text-neutral-300"}`}
                                >
                                    Entities
                                </button>
                                <button
                                    onClick={() => setAssetCategory("environment")}
                                    className={`flex-1 py-1.5 text-[10px] font-bold uppercase tracking-wider rounded-lg transition-all ${assetCategory === "environment" ? "bg-blue-600 text-white shadow-lg" : "text-neutral-500 hover:text-neutral-300"}`}
                                >
                                    Environment
                                </button>
                                <button
                                    onClick={() => setIsAssetModalOpen(true)}
                                    className="ml-2 p-1.5 bg-neutral-800 hover:bg-neutral-700 rounded-lg text-blue-400 border border-neutral-700 transition-all hover:scale-105"
                                    title="Add New Asset"
                                >
                                    <Plus size={16} />
                                </button>
                            </div>

                            {assetCategory === "entities" ? (
                                <>
                                    <TilePalette
                                        selectedTileId={selectedTileId}
                                        onSelectTile={(id) => {
                                            setSelectedTileId(id);
                                            setActiveTool("brush");
                                        }}
                                        customTiles={registryAssets
                                            .filter(a => a.type === 'tilemap')
                                            .map((a, idx) => ({ id: 10 + idx, name: a.name, image: a.image_url }))
                                        }
                                    />
                                    <section className="pt-6 border-t border-neutral-900">
                                        <h4 className="text-[10px] font-bold text-neutral-600 uppercase tracking-widest mb-3 px-1 flex justify-between">
                                            <span>Quick Spawn</span>
                                        </h4>
                                        <div className="grid grid-cols-2 gap-2">
                                            <AssetCard icon={ImageIcon} label="Character" onClick={() => handleQuickSpawn('character')} active={activeTool === "character"} />
                                            <AssetCard icon={ImageIcon} label="Monster" onClick={() => handleQuickSpawn('monster')} active={activeTool === "monster"} />

                                            {/* Custom Registry Entities - Exclude variants of tilemap */}
                                            {registryAssets.filter(a =>
                                                a.type?.toLowerCase() !== 'environment' &&
                                                !a.type?.toLowerCase().includes('tile')
                                            ).map(asset => (
                                                <AssetCard
                                                    key={asset.id}
                                                    icon={ImageIcon}
                                                    label={asset.name}
                                                    onClick={() => handleQuickSpawn(asset.name)}
                                                    active={activeTool === asset.name}
                                                    preview={asset.image_url}
                                                />
                                            ))}
                                        </div>
                                    </section>
                                </>
                            ) : (
                                <section className="space-y-4">
                                    <h4 className="text-[10px] font-bold text-neutral-600 uppercase tracking-widest mb-3 px-1">World Objects</h4>
                                    <div className="grid grid-cols-2 gap-2">
                                        <AssetCard icon={ImageIcon} label="Tree" onClick={() => handleQuickSpawn('tree')} active={activeTool === "tree"} />
                                        <AssetCard icon={ImageIcon} label="Rock" onClick={() => handleQuickSpawn('rock')} active={activeTool === "rock"} />
                                        <AssetCard icon={ImageIcon} label="Bush" onClick={() => handleQuickSpawn('bush')} active={activeTool === "bush"} />
                                        <AssetCard icon={ImageIcon} label="Bridge" onClick={() => handleQuickSpawn('bridge')} active={activeTool === "bridge"} />

                                        {/* Custom Registry Environment - Exclude variants of tilemap */}
                                        {registryAssets.filter(a =>
                                            a.type?.toLowerCase() === 'environment' &&
                                            !a.type?.toLowerCase().includes('tile')
                                        ).map(asset => (
                                            <AssetCard
                                                key={asset.id}
                                                icon={ImageIcon}
                                                label={asset.name}
                                                onClick={() => handleQuickSpawn(asset.name)}
                                                active={activeTool === asset.name}
                                                preview={asset.image_url}
                                            />
                                        ))}
                                    </div>
                                </section>
                            )}
                        </div>
                    ) : (
                        <div className="space-y-4">
                            <h4 className="text-[10px] font-bold text-neutral-600 uppercase tracking-widest px-1">Entities ({entities.length})</h4>
                            <div className="space-y-1">
                                {entities.map(ent => (
                                    <div
                                        key={ent.id}
                                        className={`group w-full flex items-center justify-between px-3 py-2 rounded-lg text-xs transition-colors ${selectedEntityId === ent.id ? 'bg-blue-600/20 text-blue-400 border border-blue-500/30' : 'text-neutral-500 hover:bg-neutral-900 border border-transparent'}`}
                                    >
                                        <button
                                            onClick={() => { setSelectedEntityId(ent.id); setSelectedTile(null); }}
                                            className="flex-1 flex items-center space-x-3 text-left"
                                        >
                                            <div className="w-2 h-2 rounded-full bg-blue-500" />
                                            <span>{ent.name}</span>
                                        </button>
                                        <div className="flex items-center space-x-2">
                                            <span className="text-[9px] uppercase opacity-30">{ent.type}</span>
                                            <button
                                                onClick={(e) => {
                                                    e.stopPropagation();
                                                    setEntities(prev => prev.filter(o => o.id !== ent.id));
                                                    if (selectedEntityId === ent.id) setSelectedEntityId(null);
                                                    setSyncStatus("pending");
                                                }}
                                                className="p-1 hover:bg-red-500/20 hover:text-red-400 rounded transition-all opacity-0 group-hover:opacity-100"
                                            >
                                                <Trash2 size={12} />
                                            </button>
                                        </div>
                                    </div>
                                ))}
                            </div>

                            <h4 className="text-[10px] font-bold text-neutral-600 uppercase tracking-widest px-1 pt-4">Placed Tiles</h4>
                            <div className="space-y-1">
                                {tilemapRef.current.grid.map((row, y) =>
                                    row.map((tileId, x) => {
                                        if (tileId === 0) return null;
                                        const type = tilemapRef.current.tileTypes[tileId];
                                        const uniqueKey = `tile-${x}-${y}`;
                                        const isSelected = selectedTile?.x === x && selectedTile?.y === y;
                                        return (
                                            <div
                                                key={uniqueKey}
                                                className={`group w-full flex items-center justify-between px-3 py-2 rounded-lg text-xs transition-colors ${isSelected ? 'bg-green-600/20 text-green-400 border border-green-500/30' : 'text-neutral-500 hover:bg-neutral-900 border border-transparent'}`}
                                            >
                                                <button
                                                    onClick={() => { setSelectedTile({ x, y, id: tileId, name: type?.name || 'Tile' }); setSelectedEntityId(null); }}
                                                    className="flex-1 flex items-center space-x-3 text-left"
                                                >
                                                    <div className="w-2 h-2 rounded-full bg-green-500" />
                                                    <span>{type?.name || 'Tile'} [{x},{y}]</span>
                                                </button>
                                                <button
                                                    onClick={(e) => {
                                                        e.stopPropagation();
                                                        tilemapRef.current.setTile(x, y, 0);
                                                        if (isSelected) setSelectedTile(null);
                                                        setSyncStatus("pending");
                                                    }}
                                                    className="p-1 hover:bg-red-500/20 hover:text-red-400 rounded transition-all opacity-0 group-hover:opacity-100"
                                                >
                                                    <Trash2 size={12} />
                                                </button>
                                            </div>
                                        );
                                    })
                                )}
                            </div>
                        </div>
                    )}
                </div>
            </aside>

            {/* 2. CENTER */}
            <main className="flex-1 flex flex-col relative">
                <header className="h-14 border-b border-neutral-900 flex items-center justify-between px-6 bg-neutral-950/80 backdrop-blur-md sticky top-0 z-10">
                    <div className="flex items-center space-x-1 bg-neutral-900/50 p-1 rounded-lg border border-neutral-800">
                        <ToolbarIcon icon={MousePointer2} active={activeTool === "select"} onClick={() => setActiveTool("select")} />
                        <ToolbarIcon icon={Brush} active={activeTool === "brush"} onClick={() => setActiveTool("brush")} />
                        <ToolbarIcon icon={Eraser} active={activeTool === "eraser"} onClick={() => setActiveTool("eraser")} />
                    </div>

                    <div className="flex items-center space-x-3">
                        <div className="flex items-center gap-3 mr-4 py-1 px-3 bg-neutral-900 rounded-full border border-neutral-800">
                            <span className={`w-1.5 h-1.5 rounded-full ${engineStatus === 'processing' ? 'bg-yellow-500 animate-pulse' :
                                engineStatus === 'connected' ? 'bg-green-500' : 'bg-red-500'
                                }`} />
                            <span className={`text-[9px] uppercase font-bold tracking-widest ${engineStatus === 'connected' ? 'text-green-400' : 'text-neutral-500'}`}>
                                Status: {engineStatus === 'connected' ? 'READY' : engineStatus.toUpperCase()}
                            </span>
                        </div>

                        <div className="flex items-center gap-3 mr-4 py-1 px-3 bg-neutral-900 rounded-full border border-neutral-800">
                            <span className={`w-1.5 h-1.5 rounded-full ${syncStatus === 'syncing' ? 'bg-yellow-500 animate-pulse' :
                                syncStatus === 'saved' ? 'bg-green-500' :
                                    syncStatus === 'error' ? 'bg-red-500' : 'bg-neutral-700'
                                }`} />
                            <span className="text-[9px] uppercase font-bold tracking-widest text-neutral-500">
                                {syncStatus.toUpperCase()}
                            </span>
                        </div>

                        <button className="p-2 hover:bg-neutral-800 rounded-lg text-neutral-400 transition-colors">
                            <Play size={18} />
                        </button>
                        <button
                            onClick={saveProject}
                            className="bg-blue-600 hover:bg-blue-500 px-4 py-1.5 rounded-lg text-xs font-bold transition-all flex items-center gap-2"
                        >
                            <Save size={14} />
                            Save
                        </button>
                    </div>
                </header>

                <div className="flex-1 flex items-center justify-center p-12 overflow-hidden bg-[radial-gradient(#1a1a1a_1px,transparent_1px)] [background-size:32px_32px]">
                    <div className="relative group bg-neutral-900 rounded-3xl overflow-hidden shadow-2xl border border-neutral-800">
                        <canvas
                            ref={canvasRef}
                            width={800}
                            height={600}
                            onMouseDown={(e) => handleCanvasInteraction(e)}
                            onMouseMove={(e) => handleCanvasInteraction(e)}
                            className="block cursor-crosshair"
                        />
                        <div className="absolute top-4 left-4 z-10 bg-black/50 backdrop-blur px-3 py-1 rounded-full border border-white/10 text-[10px] font-mono text-neutral-400">
                            WebGL2 Renderer | 60 FPS
                        </div>
                    </div>
                </div>

                {/* AI SYNC BAR - Restored */}
                <div className="absolute bottom-8 left-1/2 -translate-x-1/2 w-full max-w-xl px-4">
                    <div className="bg-neutral-900/80 backdrop-blur-xl border border-neutral-800 p-2 rounded-2xl shadow-2xl flex items-center space-x-2">
                        <input
                            type="text"
                            placeholder="Describe your scene to AI (e.g., 4 characters in a forest)..."
                            value={aiPrompt}
                            onChange={(e) => setAiPrompt(e.target.value)}
                            onKeyDown={(e) => e.key === 'Enter' && callEngineBridge("GENERATE_SCENE", { prompt: aiPrompt })}
                            className="flex-1 bg-transparent border-none focus:ring-0 px-4 text-sm"
                        />
                        <button
                            onClick={() => callEngineBridge("GENERATE_SCENE", { prompt: aiPrompt })}
                            className="bg-blue-600 p-2 px-4 rounded-xl text-sm font-bold hover:bg-blue-500 transition-all active:scale-95 flex items-center gap-2"
                        >
                            <Zap size={14} />
                            Sync AI
                        </button>
                    </div>
                </div>
            </main>

            {/* 3. RIGHT SIDEBAR */}
            <aside className="w-72 border-l border-neutral-900 bg-neutral-950/50 backdrop-blur-xl flex flex-col">
                <header className="h-14 border-b border-neutral-900 flex items-center px-4 justify-between">
                    <span className="text-sm font-semibold text-neutral-300">Inspector</span>
                    <Settings size={16} className="text-neutral-500" />
                </header>

                <div className="flex-1 overflow-y-auto p-5 custom-scrollbar space-y-8">
                    {selectedEntity ? (
                        <section>
                            <div className="flex items-center space-x-2 text-blue-400 mb-4">
                                <Database size={14} />
                                <span className="text-xs font-bold uppercase tracking-wider text-neutral-400">Transform</span>
                            </div>

                            <div className="space-y-4">
                                <PropertyRow
                                    label="Name"
                                    value={selectedEntity.name}
                                    onChange={(val) => {
                                        const newEnts = entities.map(e => e.id === selectedEntityId ? { ...e, name: val } : e);
                                        setEntities(newEnts);
                                        setSyncStatus("pending");
                                    }}
                                />
                                <PropertyRow
                                    label="Type"
                                    value={selectedEntity.type || "unknown"}
                                    onChange={(val) => {
                                        const newEnts = entities.map(e => e.id === selectedEntityId ? { ...e, type: val } : e);
                                        setEntities(newEnts);
                                        setSyncStatus("pending");
                                    }}
                                />
                                <div className="grid grid-cols-2 gap-4">
                                    <PropertyRow
                                        label="X"
                                        value={selectedEntity.x}
                                        onChange={(val) => {
                                            const newEnts = entities.map(e => e.id === selectedEntityId ? { ...e, x: parseInt(val) || 0 } : e);
                                            setEntities(newEnts);
                                            setSyncStatus("pending");
                                        }}
                                    />
                                    <PropertyRow
                                        label="Y"
                                        value={selectedEntity.y}
                                        onChange={(val) => {
                                            const newEnts = entities.map(e => e.id === selectedEntityId ? { ...e, y: parseInt(val) || 0 } : e);
                                            setEntities(newEnts);
                                            setSyncStatus("pending");
                                        }}
                                    />
                                </div>

                                <button
                                    onClick={() => {
                                        setEntities(prev => prev.filter(o => o.id !== selectedEntityId));
                                        setSelectedEntityId(null);
                                        setSyncStatus("pending");
                                    }}
                                    className="w-full bg-red-600/10 hover:bg-red-600/20 text-red-400 border border-red-500/30 rounded-xl py-2.5 text-[10px] font-bold uppercase tracking-widest transition-all active:scale-95 flex items-center justify-center gap-2 mt-4"
                                >
                                    <Trash2 size={12} />
                                    Remove Entity
                                </button>
                            </div>

                            {/* Permanent Edit Button for Registry Assets */}
                            {registryAssets.some(a => a.name === selectedEntity.name || entities.find(e => e.id === selectedEntityId)?.image_url) && (
                                <div className="mt-6 pt-6 border-t border-neutral-900">
                                    <button
                                        onClick={() => handleUpdateRegistry(selectedEntity)}
                                        className="w-full bg-blue-600/10 hover:bg-blue-600/20 text-blue-400 border border-blue-500/30 rounded-xl py-2.5 text-[10px] font-bold uppercase tracking-widest transition-all active:scale-95 flex items-center justify-center gap-2"
                                    >
                                        <Save size={12} />
                                        Save Changes (Permanent)
                                    </button>
                                    <p className="text-[9px] text-neutral-600 mt-2 text-center italic">Updates this asset in your global registry</p>
                                </div>
                            )}
                        </section>
                    ) : selectedTile ? (
                        <section>
                            <div className="flex items-center space-x-2 text-green-400 mb-4">
                                <Database size={14} />
                                <span className="text-xs font-bold uppercase tracking-wider text-neutral-400">Tile Info</span>
                            </div>

                            <div className="space-y-4">
                                <PropertyRow
                                    label="Name"
                                    value={selectedTile.name}
                                    onChange={(val) => {
                                        setSelectedTile(prev => ({ ...prev, name: val }));
                                    }}
                                />
                                <div className="grid grid-cols-2 gap-4">
                                    <div className="space-y-1">
                                        <span className="text-[9px] text-neutral-600 uppercase font-bold">Grid X</span>
                                        <div className="bg-neutral-900 px-3 py-2 rounded-lg text-xs text-neutral-400 border border-neutral-800">{selectedTile.x}</div>
                                    </div>
                                    <div className="space-y-1">
                                        <span className="text-[9px] text-neutral-600 uppercase font-bold">Grid Y</span>
                                        <div className="bg-neutral-900 px-3 py-2 rounded-lg text-xs text-neutral-400 border border-neutral-800">{selectedTile.y}</div>
                                    </div>
                                </div>

                                <button
                                    onClick={() => {
                                        tilemapRef.current.setTile(selectedTile.x, selectedTile.y, 0);
                                        setSelectedTile(null);
                                        setSyncStatus("pending");
                                    }}
                                    className="w-full bg-red-600/10 hover:bg-red-600/20 text-red-400 border border-red-500/30 rounded-xl py-2.5 text-[10px] font-bold uppercase tracking-widest transition-all active:scale-95 flex items-center justify-center gap-2"
                                >
                                    <Trash2 size={12} />
                                    Remove Tile
                                </button>
                            </div>

                            {/* Permanent Edit Button for Tile Assets */}
                            {selectedTile.id >= 10 && (
                                <div className="mt-6 pt-6 border-t border-neutral-900">
                                    <button
                                        onClick={() => {
                                            const originalAsset = registryAssets[selectedTile.id - 10];
                                            if (originalAsset) {
                                                handleUpdateRegistry({ ...originalAsset, name: selectedTile.name });
                                            }
                                        }}
                                        className="w-full bg-blue-600/10 hover:bg-blue-600/20 text-blue-400 border border-blue-500/30 rounded-xl py-2.5 text-[10px] font-bold uppercase tracking-widest transition-all active:scale-95 flex items-center justify-center gap-2"
                                    >
                                        <Save size={12} />
                                        Save Asset (Permanent)
                                    </button>
                                </div>
                            )}
                        </section>
                    ) : (
                        <div className="h-full flex flex-col items-center justify-center text-center opacity-30 select-none">
                            <MousePointer2 size={32} className="mb-2" />
                            <p className="text-xs">Select an entity to inspect</p>
                        </div>
                    )}

                    {/* Rendering Options */}
                    <section className="pt-6 border-t border-neutral-900">
                        <h4 className="text-[10px] font-bold text-neutral-600 uppercase tracking-widest mb-4">Rendering (WebGL2)</h4>
                        <div className="space-y-3">
                            <div className="flex justify-between items-center text-xs text-neutral-400">
                                <span>Batch Size</span>
                                <span className="font-mono">1000 SPR</span>
                            </div>
                            <div className="flex justify-between items-center text-xs text-neutral-400">
                                <span>Draw Calls</span>
                                <span className="font-mono">1-2 DC</span>
                            </div>
                        </div>
                    </section>
                </div>
            </aside>

            <style jsx global>{`
                .custom-scrollbar::-webkit-scrollbar { width: 5px; }
                .custom-scrollbar::-webkit-scrollbar-track { background: transparent; }
                .custom-scrollbar::-webkit-scrollbar-thumb { background: #262626; border-radius: 10px; }
            `}</style>

            <AssetCreationModal
                isOpen={isAssetModalOpen}
                onClose={() => setIsAssetModalOpen(false)}
                onAssetCreated={fetchRegistryAssets}
            />
        </div>
    );
}

function TabButton({ active, onClick, icon: Icon, label }) {
    return (
        <button
            onClick={onClick}
            className={`flex-1 flex flex-col items-center py-3 space-y-1 transition-all relative ${active ? 'text-blue-500' : 'text-neutral-600 hover:text-neutral-400'}`}
        >
            <Icon size={16} />
            <span className="text-[9px] font-bold uppercase">{label}</span>
            {active && <motion.div layoutId="tab-active" className="absolute bottom-0 left-0 right-0 h-0.5 bg-blue-500" />}
        </button>
    );
}

function AssetCard({ icon: Icon, label, onClick, active, preview }) {
    return (
        <button
            onClick={onClick}
            className={`bg-neutral-900 border rounded-lg p-3 transition-all cursor-pointer group text-left w-full ${active ? 'border-blue-500 bg-blue-600/10' : 'border-neutral-800 hover:border-blue-500/50 hover:bg-neutral-800'}`}
        >
            <div className={`aspect-square rounded mb-2 flex items-center justify-center border transition-colors overflow-hidden ${active ? 'bg-blue-600/20 border-blue-500/50' : 'bg-neutral-950 border-white/5 group-hover:bg-blue-600/10'}`}>
                {preview ? (
                    <img src={preview} alt={label} className="w-full h-full object-contain p-1" />
                ) : (
                    <Icon size={20} className={active ? "text-blue-400" : "text-neutral-700 group-hover:text-blue-500"} />
                )}
            </div>
            <p className={`text-[10px] truncate text-center font-medium ${active ? 'text-blue-400' : 'text-neutral-500 group-hover:text-neutral-300'}`}>{label}</p>
        </button>
    );
}

function ToolbarIcon({ icon: Icon, active = false, onClick }) {
    return (
        <button
            onClick={onClick}
            className={`p-2 rounded-md transition-all ${active ? 'bg-blue-600 text-white shadow-lg shadow-blue-600/20' : 'text-neutral-500 hover:bg-neutral-800 hover:text-neutral-300'}`}
        >
            <Icon size={16} />
        </button>
    );
}

function PropertyRow({ label, value, onChange }) {
    return (
        <div className="flex items-center justify-between space-x-4">
            <label className="text-xs text-neutral-500 font-medium">{label}</label>
            <input
                type="text"
                value={value}
                onChange={(e) => onChange(e.target.value)}
                className="w-24 bg-neutral-900 border border-neutral-800 rounded px-2 py-1 text-xs text-neutral-300 focus:outline-none focus:border-blue-500/50"
            />
        </div>
    );
}
