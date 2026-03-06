"use client";

import React, { useState, useEffect } from 'react';
import { supabase } from '@/lib/supabase';

const TILE_TYPES = [
    { id: 0, name: 'Soil', color: '#8B6B4F' },
    { id: 1, name: 'Wall', color: '#666666' },
    { id: 2, name: 'Water', color: '#3380CC' }
];

export default function TilePalette({ selectedTileId, onSelectTile }) {
    const [customTiles, setCustomTiles] = useState([]);

    useEffect(() => {
        const fetchCustomTiles = async () => {
            try {
                const { data, error } = await supabase
                    .from('assets_registry')
                    .select('*')
                    .or('type.eq.tilemap,type.eq.TILE MAP,type.ilike.%Tile%Terrain%')
                    .order('created_at', { ascending: false });

                if (error) throw error;

                const mappedTiles = (data || []).map((asset, index) => ({
                    id: 10 + index, // Custom IDs start at 10
                    name: asset.name,
                    image: asset.image_url,
                    spriteName: asset.name // Ensure sprite lookup works
                }));

                setCustomTiles(mappedTiles);
            } catch (err) {
                console.error("[TilePalette] Fetch error:", err);
            }
        };

        fetchCustomTiles();
    }, []);

    const allTiles = [...TILE_TYPES, ...customTiles];

    return (
        <div className="bg-neutral-900/50 p-6 rounded-3xl border border-neutral-800 backdrop-blur-sm">
            <h3 className="text-[10px] font-bold text-neutral-500 uppercase tracking-widest mb-4">Tile Palette</h3>
            <div className="grid grid-cols-3 gap-3">
                {allTiles.map((tile) => (
                    <button
                        key={tile.id}
                        onClick={() => onSelectTile(tile.id)}
                        className={`group relative flex flex-col items-center gap-2 p-2 rounded-xl transition-all ${selectedTileId === tile.id
                            ? 'bg-blue-600/20 border-blue-600/50 scale-105 shadow-lg shadow-blue-600/10'
                            : 'bg-neutral-950 border-neutral-800 hover:border-neutral-700'
                            } border`}
                    >
                        <div
                            className="w-10 h-10 rounded-lg shadow-inner overflow-hidden flex items-center justify-center border border-white/5 bg-neutral-900"
                            style={{ backgroundColor: tile.color || 'transparent' }}
                        >
                            {tile.image ? (
                                <img
                                    src={tile.image}
                                    alt={tile.name}
                                    className="w-full h-full object-cover"
                                    onError={(e) => { e.target.style.display = 'none'; }}
                                />
                            ) : null}
                        </div>
                        <span className="text-[9px] uppercase tracking-tighter text-neutral-500 group-hover:text-neutral-300 truncate w-full px-1 text-center font-bold">
                            {tile.name}
                        </span>
                        {selectedTileId === tile.id && (
                            <div className="absolute -top-1 -right-1 w-3 h-3 bg-blue-500 rounded-full border-2 border-neutral-900 shadow-sm" />
                        )}
                    </button>
                ))}
            </div>

            <div className="mt-6 p-4 bg-neutral-950/50 rounded-2xl border border-neutral-800">
                <p className="text-[10px] text-neutral-600 leading-relaxed italic text-center">
                    Select a tile and paint on the canvas.
                </p>
            </div>
        </div>
    );
}
