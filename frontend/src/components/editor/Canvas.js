"use client";
import { useEffect, useRef, useState } from "react";

export default function Canvas({ width = 800, height = 600, objects = [], selectedId = null }) {
    const canvasRef = useRef(null);
    const [frameCount, setFrameCount] = useState(0);

    useEffect(() => {
        const canvas = canvasRef.current;
        if (!canvas) return;

        const ctx = canvas.getContext("2d");
        let animationFrameId;

        const render = () => {
            setFrameCount((prev) => prev + 1);

            // Clear canvas
            ctx.clearRect(0, 0, width, height);

            // Draw Grid
            ctx.strokeStyle = "#1a1a1a";
            ctx.lineWidth = 1;
            const gridSize = 40;

            for (let x = 0; x <= width; x += gridSize) {
                ctx.beginPath();
                ctx.moveTo(x, 0);
                ctx.lineTo(x, height);
                ctx.stroke();
            }

            for (let y = 0; y <= height; y += gridSize) {
                ctx.beginPath();
                ctx.moveTo(0, y);
                ctx.lineTo(width, y);
                ctx.stroke();
            }

            // Draw Objects
            objects.forEach((obj) => {
                const isSelected = obj.id === selectedId;

                // Draw Shadow/Glow if selected
                if (isSelected) {
                    ctx.shadowBlur = 15;
                    ctx.shadowColor = "#3b82f6";
                    ctx.strokeStyle = "#3b82f6";
                    ctx.lineWidth = 2;
                    ctx.strokeRect(obj.x - 22, obj.y - 22, 44, 44);
                } else {
                    ctx.shadowBlur = 0;
                }

                // Draw Object (Circle for now, representing Sprite)
                ctx.fillStyle = obj.color || "#3b82f6";
                ctx.beginPath();
                ctx.arc(obj.x, obj.y, 20, 0, Math.PI * 2);
                ctx.fill();

                // Draw Label
                ctx.shadowBlur = 0;
                ctx.fillStyle = "white";
                ctx.font = "10px Inter";
                ctx.textAlign = "center";
                ctx.fillText(obj.name.toUpperCase(), obj.x, obj.y + 35);
            });

            animationFrameId = window.requestAnimationFrame(render);
        };

        render();

        return () => {
            window.cancelAnimationFrame(animationFrameId);
        };
    }, [width, height, objects, selectedId]);

    return (
        <div className="relative group bg-neutral-900 rounded-xl overflow-hidden shadow-2xl border border-neutral-800">
            <div className="absolute top-4 left-4 z-10 bg-black/50 backdrop-blur px-3 py-1 rounded-full border border-white/10 text-[10px] font-mono text-neutral-400">
                FPS: 60 | Frames: {frameCount}
            </div>
            <canvas
                ref={canvasRef}
                width={width}
                height={height}
                className="block cursor-crosshair active:cursor-grabbing"
            />
        </div>
    );
}
