"use client";
import { useState, useEffect, useRef } from "react";
import { X, Upload, ImageIcon, Loader2 } from "lucide-react";
import { motion, AnimatePresence } from "framer-motion";
import { supabase } from "@/lib/supabase";

export default function AssetCreationModal({ isOpen, onClose, onAssetCreated }) {
    const [name, setName] = useState("");
    const [type, setType] = useState("character");
    const [file, setFile] = useState(null);
    const [previewUrl, setPreviewUrl] = useState(null);
    const [isUploading, setIsUploading] = useState(false);
    const fileInputRef = useRef(null);

    // Reset form when modal opens
    useEffect(() => {
        if (isOpen) {
            setName("");
            setType("character");
            setFile(null);
            setPreviewUrl(null);
            setIsUploading(false);
        }
    }, [isOpen]);

    const handleFileChange = (e) => {
        const selectedFile = e.target.files[0];
        if (selectedFile) {
            setFile(selectedFile);
            const reader = new FileReader();
            reader.onloadend = () => setPreviewUrl(reader.result);
            reader.readAsDataURL(selectedFile);
        }
    };

    const handleCreate = async () => {
        if (!name || !file) {
            alert("Please fill in the name and select an image.");
            return;
        }

        setIsUploading(true);
        try {
            // 1. Get User ID (Optional if RLS is disabled)
            const { data: { user } } = await supabase.auth.getUser();

            // 2. Upload to Supabase Storage
            const fileExt = file.name.split('.').pop();
            const fileName = `${Date.now()}.${fileExt}`;
            // Simpler path for global registry
            const filePath = `public/${fileName}`;

            const { error: uploadError } = await supabase.storage
                .from('assets')
                .upload(filePath, file, {
                    cacheControl: '3600',
                    upsert: true
                });

            if (uploadError) throw uploadError;

            // 3. Get Public URL
            const { data: { publicUrl } } = supabase.storage
                .from('assets')
                .getPublicUrl(filePath);

            // 4. Save to Database Registry
            const assetData = {
                name: name,
                type: type,
                image_url: publicUrl
            };

            // Only add user_id if authenticated, but don't fail if not
            if (user) {
                assetData.user_id = user.id;
            }

            if (dbError) throw dbError;

            // ─── Bridge: Sync to C Engine ─────────────────────────────────────
            // Ambil semua data assets untuk Sync Shared State ke Mesin Farel
            const { data: allAssets } = await supabase
                .from('assets_registry')
                .select('*');

            try {
                const bridgeResponse = await fetch(`${process.env.NEXT_PUBLIC_BACKEND_URL || 'http://localhost:3001'}/api/engine/create`, {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({
                        assets: allAssets, // Shared State Sync
                        name: name,
                        type: type,
                        image_url: publicUrl
                    })
                });
                const bridgeData = await bridgeResponse.json();
                console.log("[Bridge] C Engine Response:", bridgeData);
            } catch (bridgeErr) {
                console.error("[Bridge] Failed to invoke C Engine:", bridgeErr);
                // Kita tidak fail-kan upload Supabase jika Engine gagal, cuma log saja
            }
            // ──────────────────────────────────────────────────────────────────

            // 5. Success cleanup
            setIsUploading(false);
            onAssetCreated();
            onClose();

            // Reset local states
            setName("");
            setType("character");
            setFile(null);
            setPreviewUrl(null);

        } catch (error) {
            console.error("Asset Creation Error Track:", error);
            alert("System Message: " + (error.message || "Unknown error occurred"));
            setIsUploading(false);
        }
    };

    return (
        <AnimatePresence>
            {isOpen && (
                <div className="fixed inset-0 z-[100] flex items-center justify-center p-4">
                    <motion.div
                        initial={{ opacity: 0 }}
                        animate={{ opacity: 1 }}
                        exit={{ opacity: 0 }}
                        onClick={onClose}
                        className="absolute inset-0 bg-black/60 backdrop-blur-sm"
                    />

                    <motion.div
                        initial={{ opacity: 0, scale: 0.95, y: 20 }}
                        animate={{ opacity: 1, scale: 1, y: 0 }}
                        exit={{ opacity: 0, scale: 0.95, y: 20 }}
                        className="relative w-full max-w-md bg-neutral-900 border border-neutral-800 rounded-2xl shadow-2xl overflow-hidden"
                    >
                        <header className="p-4 border-b border-neutral-800 flex items-center justify-between">
                            <h3 className="text-sm font-bold uppercase tracking-widest text-neutral-400">Create New Asset</h3>
                            <button onClick={onClose} className="p-1 hover:bg-neutral-800 rounded-md text-neutral-500">
                                <X size={18} />
                            </button>
                        </header>

                        <div className="p-6 space-y-6">
                            {/* Name Input */}
                            <div className="space-y-2">
                                <label className="text-[10px] font-bold text-neutral-500 uppercase tracking-wider">Asset Name</label>
                                <input
                                    type="text"
                                    value={name}
                                    onChange={(e) => setName(e.target.value)}
                                    placeholder="Enter character/object name..."
                                    className="w-full bg-neutral-950 border border-neutral-800 rounded-xl px-4 py-3 text-sm focus:outline-none focus:border-blue-500 transition-all"
                                />
                            </div>

                            {/* Type Dropdown */}
                            <div className="space-y-2">
                                <label className="text-[10px] font-bold text-neutral-500 uppercase tracking-wider">Category</label>
                                <select
                                    value={type}
                                    onChange={(e) => setType(e.target.value)}
                                    className="w-full bg-neutral-950 border border-neutral-800 rounded-xl px-4 py-3 text-sm focus:outline-none focus:border-blue-500 appearance-none transition-all"
                                >
                                    <option value="ENTITY">Character / NPC / Monster</option>
                                    <option value="environment">Environment / Object</option>
                                    <option value="Tile / Terrain (Tilemap)">Tile / Terrain (Tilemap)</option>
                                </select>
                            </div>

                            {/* Upload Area */}
                            <div className="space-y-2">
                                <label className="text-[10px] font-bold text-neutral-500 uppercase tracking-wider">Sprite Image</label>
                                <div
                                    onClick={() => fileInputRef.current?.click()}
                                    className="relative aspect-video rounded-xl border-2 border-dashed border-neutral-800 bg-neutral-950 hover:bg-neutral-900 hover:border-blue-500/50 cursor-pointer flex flex-col items-center justify-center transition-all group overflow-hidden"
                                >
                                    {previewUrl ? (
                                        <img src={previewUrl} alt="Preview" className="h-full w-full object-contain p-4" />
                                    ) : (
                                        <>
                                            <Upload className="text-neutral-700 group-hover:text-blue-500 mb-2 transition-colors" size={32} />
                                            <p className="text-xs text-neutral-500 group-hover:text-neutral-300 transition-colors">Click or drag image to upload</p>
                                        </>
                                    )}
                                    <input
                                        type="file"
                                        ref={fileInputRef}
                                        onChange={handleFileChange}
                                        accept="image/*"
                                        className="hidden"
                                    />
                                </div>
                            </div>
                        </div>

                        <footer className="p-4 bg-neutral-950 border-t border-neutral-800 flex justify-end space-x-3">
                            <button
                                onClick={onClose}
                                className="px-5 py-2 text-xs font-bold text-neutral-500 hover:text-neutral-300 transition-colors"
                            >
                                CANCEL
                            </button>
                            <button
                                onClick={handleCreate}
                                disabled={isUploading || !name || !file}
                                className="bg-blue-600 hover:bg-blue-500 disabled:opacity-50 disabled:cursor-not-allowed px-6 py-2 rounded-xl text-xs font-bold flex items-center gap-2 transition-all active:scale-95"
                            >
                                {isUploading ? (
                                    <>
                                        <Loader2 size={14} className="animate-spin" />
                                        CREATING...
                                    </>
                                ) : (
                                    <>
                                        <ImageIcon size={14} />
                                        CREATE DATA
                                    </>
                                )}
                            </button>
                        </footer>
                    </motion.div>
                </div>
            )}
        </AnimatePresence>
    );
}
