"use client";
import { useState, useEffect, useMemo } from "react";
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
    Square
} from "lucide-react";
import Link from "next/link";
import dynamic from "next/dynamic";
import { motion, AnimatePresence } from "framer-motion";

const Canvas = dynamic(() => import("@/components/editor/Canvas"), {
    ssr: false,
    loading: () => (
        <div className="w-[800px] h-[600px] bg-neutral-900/50 rounded-2xl flex items-center justify-center border border-neutral-800">
            <span className="text-xs text-neutral-500 font-mono animate-pulse">Initializing Canvas...</span>
        </div>
    )
});

export default function EditorPage({ params }) {
    const [projectId, setProjectId] = useState(null);
    const [projectName, setProjectName] = useState("Loading...");
    const [activeSidebar, setActiveSidebar] = useState("assets");
    const [engineStatus, setEngineStatus] = useState("idle");

    // Scene State
    const [sceneObjects, setSceneObjects] = useState([
        { id: "obj-1", name: "Main_Character", x: 400, y: 300, color: "#3b82f6" }
    ]);
    const [selectedObjectId, setSelectedObjectId] = useState("obj-1");

    const selectedObject = useMemo(() =>
        sceneObjects.find(obj => obj.id === selectedObjectId),
        [sceneObjects, selectedObjectId]
    );

    useEffect(() => {
        const resolveParams = async () => {
            const resolvedParams = await params;
            setProjectId(resolvedParams.id);

            const stored = localStorage.getItem("regmcraft_projects");
            if (stored) {
                const projects = JSON.parse(stored);
                // Update timestamp logic
                const updatedProjects = projects.map(p => {
                    if (p.id === resolvedParams.id) {
                        setProjectName(p.name);
                        return { ...p, lastModified: new Date().toISOString() };
                    }
                    return p;
                });
                localStorage.setItem("regmcraft_projects", JSON.stringify(updatedProjects));
            }
        };
        resolveParams();
    }, [params]);

    // Spawn Logic
    const handleSpawnObject = (type) => {
        const newObj = {
            id: `obj-${Date.now()}`,
            name: `${type}_${sceneObjects.length + 1}`,
            x: 400 + (Math.random() - 0.5) * 100,
            y: 300 + (Math.random() - 0.5) * 100,
            color: type === 'Enemy' ? '#ef4444' : type === 'Tile' ? '#10b981' : '#3b82f6'
        };
        setSceneObjects(prev => [...prev, newObj]);
        setSelectedObjectId(newObj.id);
        console.log(`%c[Spawn] Added: ${newObj.name}`, "color: #10b981");
    };

    // Update Property Logic
    const updateObjectProperty = (id, property, value) => {
        setSceneObjects(prev => prev.map(obj => {
            if (obj.id === id) {
                const val = (property === 'x' || property === 'y') ? parseInt(value) || 0 : value;
                return { ...obj, [property]: val };
            }
            return obj;
        }));
    };

    // API Bridge Function
    const callEngineBridge = async (action, data = {}) => {
        setEngineStatus("processing");
        const payload = {
            action,
            data: { ...data, scene: sceneObjects },
            timestamp: new Date().toISOString()
        };

        console.log(`%c[Engine Bridge] Calling action: ${action}`, "color: #3b82f6; font-weight: bold", payload);

        try {
            const response = await fetch("/api/engine", {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify(payload)
            });

            const result = await response.json();
            console.log("%c[Engine Bridge] Success:", "color: #10b981; font-weight: bold", result);
            setEngineStatus("success");

            // Update timestamp on Save
            if (action === "SAVE_PROJECT") {
                const stored = localStorage.getItem("regmcraft_projects");
                if (stored) {
                    const projects = JSON.parse(stored);
                    const updatedProjects = projects.map(p => {
                        if (p.id === projectId) return { ...p, lastModified: new Date().toISOString() };
                        return p;
                    });
                    localStorage.setItem("regmcraft_projects", JSON.stringify(updatedProjects));
                }
            }

            setTimeout(() => setEngineStatus("idle"), 2000);
            return result;
        } catch (error) {
            console.error("[Engine Bridge] Error:", error);
            setEngineStatus("error");
            return null;
        }
    };

    return (
        <div className="flex h-screen bg-neutral-950 text-white overflow-hidden font-sans">
            {/* 1. LEFT SIDEBAR */}
            <aside className="w-72 border-r border-neutral-900 flex flex-col bg-neutral-950/50 backdrop-blur-xl">
                <header className="h-14 border-b border-neutral-900 flex items-center px-4 space-x-3">
                    <Link href="/dashboard" className="p-1 hover:bg-neutral-800 rounded-md text-neutral-500 transition-colors">
                        <ChevronLeft size={18} />
                    </Link>
                    <span className="text-sm font-semibold truncate text-neutral-300">{projectName}</span>
                </header>

                <nav className="flex items-center border-b border-neutral-900 px-2 bg-neutral-900/20">
                    <TabButton active={activeSidebar === "assets"} onClick={() => setActiveSidebar("assets")} icon={Box} label="Assets" />
                    <TabButton active={activeSidebar === "layers"} onClick={() => setActiveSidebar("layers")} icon={Layers} label="Hierarchy" />
                </nav>

                <div className="flex-1 overflow-y-auto p-4 custom-scrollbar">
                    {activeSidebar === 'assets' ? (
                        <div className="space-y-6">
                            <div className="relative mb-6">
                                <Search className="absolute left-3 top-1/2 -translate-y-1/2 text-neutral-600" size={14} />
                                <input
                                    type="text"
                                    placeholder="Filter assets..."
                                    className="w-full bg-neutral-900 border border-neutral-800 rounded-lg py-1.5 pl-9 pr-3 text-xs focus:outline-none focus:ring-1 focus:ring-blue-500/50"
                                />
                            </div>
                            <section>
                                <h4 className="text-[10px] font-bold text-neutral-600 uppercase tracking-widest mb-3 px-1">Sprites</h4>
                                <div className="grid grid-cols-2 gap-2">
                                    <AssetCard icon={ImageIcon} label="Player" onClick={() => handleSpawnObject('Player')} />
                                    <AssetCard icon={ImageIcon} label="Enemy" onClick={() => handleSpawnObject('Enemy')} />
                                    <AssetCard icon={ImageIcon} label="Tile" onClick={() => handleSpawnObject('Tile')} />
                                </div>
                            </section>
                        </div>
                    ) : (
                        <div className="space-y-2">
                            {sceneObjects.map(obj => (
                                <button
                                    key={obj.id}
                                    onClick={() => setSelectedObjectId(obj.id)}
                                    className={`w-full flex items-center space-x-3 px-3 py-2 rounded-lg text-xs transition-colors ${selectedObjectId === obj.id ? 'bg-blue-600/20 text-blue-400 border border-blue-500/30' : 'text-neutral-500 hover:bg-neutral-900'}`}
                                >
                                    <div className="w-2 h-2 rounded-full" style={{ backgroundColor: obj.color }} />
                                    <span>{obj.name}</span>
                                </button>
                            ))}
                        </div>
                    )}
                </div>
            </aside>

            {/* 2. CENTER */}
            <main className="flex-1 flex flex-col relative bg-[radial-gradient(#222_1px,transparent_1px)] [background-size:32px_32px]">
                <header className="h-14 border-b border-neutral-900 flex items-center justify-between px-6 bg-neutral-950/80 backdrop-blur-md sticky top-0 z-10">
                    <div className="flex items-center space-x-1 bg-neutral-900/50 p-1 rounded-lg border border-neutral-800">
                        <ToolbarIcon icon={MousePointer2} active />
                        <ToolbarIcon icon={Brush} />
                        <ToolbarIcon icon={Eraser} />
                        <ToolbarIcon icon={Type} />
                    </div>

                    <div className="flex items-center space-x-3">
                        <div className="flex items-center space-x-2 mr-4">
                            <span className={`w-2 h-2 rounded-full ${engineStatus === 'processing' ? 'bg-yellow-500 animate-pulse' : engineStatus === 'error' ? 'bg-red-500' : 'bg-green-500'}`} />
                            <span className="text-[10px] text-neutral-500 font-mono uppercase tracking-tighter">Engine: {engineStatus}</span>
                        </div>
                        <button className="p-2 hover:bg-neutral-800 rounded-lg text-neutral-400 transition-colors" title="Preview Game">
                            <Play size={18} />
                        </button>
                        <button
                            onClick={() => callEngineBridge("SAVE_PROJECT", { id: projectId, name: projectName })}
                            className="flex items-center space-x-2 bg-blue-600 hover:bg-blue-500 px-4 py-1.5 rounded-lg text-xs font-bold transition-all"
                        >
                            <Save size={16} />
                            <span>Save</span>
                        </button>
                    </div>
                </header>

                <div className="flex-1 flex items-center justify-center p-12 overflow-hidden">
                    <AnimatePresence mode="wait">
                        <motion.div
                            initial={{ opacity: 0, scale: 0.95 }}
                            animate={{ opacity: 1, scale: 1 }}
                            transition={{ duration: 0.4 }}
                            className="relative"
                        >
                            <Canvas width={800} height={600} objects={sceneObjects} selectedId={selectedObjectId} />
                        </motion.div>
                    </AnimatePresence>
                </div>

                <div className="absolute bottom-8 left-1/2 -translate-x-1/2 w-full max-w-xl px-4">
                    <div className="bg-neutral-900/80 backdrop-blur-xl border border-neutral-800 p-2 rounded-2xl shadow-2xl flex items-center space-x-2">
                        <input
                            type="text"
                            placeholder="Describe your scene to AI..."
                            className="flex-1 bg-transparent border-none focus:ring-0 px-4 text-sm"
                        />
                        <button
                            onClick={() => callEngineBridge("GENERATE_MAP", { prompt: "forest" })}
                            className="bg-blue-600 p-2 px-6 rounded-xl text-sm font-bold hover:bg-blue-500 transition-all active:scale-95"
                        >
                            Sync C-Engine
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
                    {selectedObject ? (
                        <section>
                            <div className="flex items-center space-x-2 text-blue-400 mb-4">
                                <Database size={14} />
                                <span className="text-xs font-bold uppercase tracking-wider text-neutral-400">Object Properties</span>
                            </div>

                            <div className="space-y-4">
                                <PropertyRow
                                    label="Name"
                                    value={selectedObject.name}
                                    onChange={(val) => updateObjectProperty(selectedObjectId, 'name', val)}
                                />
                                <div className="grid grid-cols-2 gap-4">
                                    <PropertyRow
                                        label="PosX"
                                        value={selectedObject.x}
                                        onChange={(val) => updateObjectProperty(selectedObjectId, 'x', val)}
                                    />
                                    <PropertyRow
                                        label="PosY"
                                        value={selectedObject.y}
                                        onChange={(val) => updateObjectProperty(selectedObjectId, 'y', val)}
                                    />
                                </div>
                            </div>
                        </section>
                    ) : (
                        <div className="h-full flex flex-col items-center justify-center text-center opacity-30 select-none">
                            <MousePointer2 size={32} className="mb-2" />
                            <p className="text-xs">Select an object to inspect</p>
                        </div>
                    )}

                    <section className="pt-6 border-t border-neutral-900">
                        <h4 className="text-[10px] font-bold text-neutral-600 uppercase tracking-widest mb-4">Rendering (WebGL2)</h4>
                        <div className="space-y-3">
                            <ToggleButton label="Cast Shadows" active />
                            <ToggleButton label="Smooth Pixels" />
                            <ToggleButton label="C-Gen Optimization" active />
                        </div>
                    </section>
                </div>

                <div className="p-4 border-t border-neutral-900 bg-black/20">
                    <div className="flex items-center justify-between text-[10px] text-neutral-500 mb-2">
                        <span>VRAM USAGE</span>
                        <span>{12 + sceneObjects.length * 0.4} MB / 2048 MB</span>
                    </div>
                    <div className="h-1 w-full bg-neutral-900 rounded-full overflow-hidden">
                        <div className="h-full bg-blue-500/50" style={{ width: `${(12 + sceneObjects.length * 0.4) / 20.48}%` }} />
                    </div>
                </div>
            </aside>

            <style jsx global>{`
        .custom-scrollbar::-webkit-scrollbar { width: 5px; }
        .custom-scrollbar::-webkit-scrollbar-track { background: transparent; }
        .custom-scrollbar::-webkit-scrollbar-thumb { background: #262626; border-radius: 10px; }
      `}</style>
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

function AssetCard({ icon: Icon, label, onClick }) {
    return (
        <button
            onClick={onClick}
            className="bg-neutral-900 border border-neutral-800 rounded-lg p-3 hover:border-blue-500/50 hover:bg-neutral-800 transition-all cursor-pointer group text-left w-full"
        >
            <div className="aspect-square bg-neutral-950 rounded mb-2 flex items-center justify-center border border-white/5 group-hover:bg-blue-600/10">
                <Icon size={20} className="text-neutral-700 group-hover:text-blue-500 transition-colors" />
            </div>
            <p className="text-[10px] text-neutral-500 truncate text-center font-medium group-hover:text-neutral-300">{label}</p>
        </button>
    );
}

function ToolbarIcon({ icon: Icon, active = false }) {
    return (
        <button className={`p-2 rounded-md transition-all ${active ? 'bg-blue-600 text-white shadow-lg shadow-blue-600/20' : 'text-neutral-500 hover:bg-neutral-800 hover:text-neutral-300'}`}>
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

function ToggleButton({ label, active = false }) {
    return (
        <div className="flex items-center justify-between">
            <span className="text-xs text-neutral-400">{label}</span>
            <button className={`w-8 h-4 rounded-full relative transition-colors ${active ? 'bg-blue-600' : 'bg-neutral-800'}`}>
                <div className={`absolute top-0.5 w-3 h-3 rounded-full bg-white transition-all ${active ? 'right-0.5' : 'left-0.5'}`} />
            </button>
        </div>
    );
}
