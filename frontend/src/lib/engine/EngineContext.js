"use client";

import React, { createContext, useContext, useState, useCallback, useEffect } from 'react';

const EngineContext = createContext(null);

export const useEngine = () => {
    const context = useContext(EngineContext);
    if (!context) {
        throw new Error('useEngine must be used within an EngineProvider');
    }
    return context;
};

export const EngineProvider = ({ children }) => {
    const [palettes, setPalettes] = useState([]);
    const [currentPaletteId, setCurrentPaletteId] = useState(0);
    const [character, setCharacter] = useState(null);
    const [monsters, setMonsters] = useState([]);
    const [vehicles, setVehicles] = useState([]);
    const [isLoading, setIsLoading] = useState(false);
    const [error, setError] = useState(null);

    const API_BASE = process.env.NEXT_PUBLIC_API_URL || 'http://localhost:3001/api/engine';

    const initEngine = useCallback(async () => {
        setIsLoading(true);
        setError(null);
        try {
            const res = await fetch(`${API_BASE}/init`, { method: 'POST' });
            const data = await res.json();
            if (data.status === 'ok') {
                setPalettes(data.palettes || []);
                setCurrentPaletteId(data.currentPaletteId || 0);
                setCharacter(data.character || null);
                setMonsters(data.monsters || []);
                setVehicles(data.vehicles || data.vehicle || []); // Handle possible singular 'vehicle'
            } else {
                setError(data.error || 'Failed to initialize engine');
            }
        } catch (err) {
            setError(err.message);
        } finally {
            setIsLoading(false);
        }
    }, [API_BASE]);

    const editEntity = useCallback(async (type, payload) => {
        setIsLoading(true);
        try {
            const res = await fetch(`${API_BASE}/edit`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ type, ...payload })
            });
            const data = await res.json();
            if (data.status === 'ok') {
                if (type === 'Character') setCharacter(data.character);
                // Handle monster/vehicle updates if needed
                return data;
            }
        } catch (err) {
            setError(err.message);
        } finally {
            setIsLoading(false);
        }
    }, [API_BASE]);

    return (
        <EngineContext.Provider value={{
            palettes,
            currentPaletteId,
            character,
            monsters,
            vehicles,
            isLoading,
            error,
            initEngine,
            editEntity,
            setCurrentPaletteId
        }}>
            {children}
        </EngineContext.Provider>
    );
};
