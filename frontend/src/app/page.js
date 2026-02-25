"use client";
import React, { useState, useEffect, useRef, useMemo, Suspense } from "react";
import Link from "next/link";
import { Canvas, useFrame, useThree } from "@react-three/fiber";
import {
  Float,
  MeshDistortMaterial,
  GradientTexture,
  PerspectiveCamera,
  OrbitControls,
  PresentationControls,
  Environment,
  ContactShadows,
  Html
} from "@react-three/drei";
import { motion, useScroll, useTransform, useSpring, useMotionValue, AnimatePresence } from "framer-motion";
import { Gamepad2, Cpu, Sparkles, Layout, ArrowRight, Zap, Boxes, MousePointer2, ChevronRight } from "lucide-react";
import * as THREE from "three";

// --- 3D Components ---

function AnimatedShape({ shape = "torus", color = "#3b82f6", index = 0 }) {
  const mesh = useRef();
  const { scrollYProgress } = useScroll();

  // Transform scroll into rotation and position
  const rotateX = useTransform(scrollYProgress, [0, 1], [0, Math.PI * 2]);
  const rotateY = useTransform(scrollYProgress, [0, 1], [0, Math.PI * 4]);
  const scale = useTransform(scrollYProgress, [0, 0.5, 1], [1, 1.5, 0.5]);

  const springScale = useSpring(scale, { damping: 20, stiffness: 100 });

  useFrame((state) => {
    const time = state.clock.getElapsedTime();
    if (mesh.current) {
      mesh.current.rotation.x = rotateX.get() + Math.sin(time * 0.5 + index) * 0.2;
      mesh.current.rotation.y = rotateY.get() + Math.cos(time * 0.3 + index) * 0.2;
      mesh.current.scale.setScalar(springScale.get());
    }
  });

  return (
    <Float speed={2} rotationIntensity={1} floatIntensity={2}>
      <mesh ref={mesh} position={[Math.sin(index) * 5, Math.cos(index) * 2, -5]}>
        {shape === "torus" && <torusKnotGeometry args={[1, 0.3, 128, 16]} />}
        {shape === "sphere" && <sphereGeometry args={[1.5, 64, 64]} />}
        {shape === "box" && <boxGeometry args={[2, 2, 2]} />}
        <MeshDistortMaterial
          color={color}
          speed={2}
          distort={0.4}
          radius={1}
          metalness={0.8}
          roughness={0.2}
          transparent
          opacity={0.6}
        />
      </mesh>
    </Float>
  );
}

function Scene3D() {
  const { scrollYProgress } = useScroll();
  const mouseX = useMotionValue(0);
  const mouseY = useMotionValue(0);

  useEffect(() => {
    const handleMouseMove = (e) => {
      mouseX.set((e.clientX / window.innerWidth) - 0.5);
      mouseY.set((e.clientY / window.innerHeight) - 0.5);
    };
    window.addEventListener("mousemove", handleMouseMove);
    return () => window.removeEventListener("mousemove", handleMouseMove);
  }, [mouseX, mouseY]);

  return (
    <div className="absolute inset-0 pointer-events-none z-0">
      <Canvas dpr={[1, 2]} camera={{ position: [0, 0, 15], fov: 45 }}>
        <color attach="background" args={["#050505"]} />
        <ambientLight intensity={0.5} />
        <spotLight position={[10, 10, 10]} angle={0.15} penumbra={1} />
        <pointLight position={[-10, -10, -10]} intensity={1} color="#3b82f6" />

        <Suspense fallback={null}>
          <group>
            <AnimatedShape shape="torus" color="#6366f1" index={0} />
            <AnimatedShape shape="sphere" color="#3b82f6" index={2} />
            <AnimatedShape shape="box" color="#a855f7" index={4} />
          </group>
          <Environment preset="city" />
          <ContactShadows position={[0, -10, 0]} opacity={0.4} scale={40} blur={2.5} far={40} />
        </Suspense>

        {/* Mouse Parallax for the whole group */}
        <SceneController mouseX={mouseX} mouseY={mouseY} />
      </Canvas>
    </div>
  );
}

function SceneController({ mouseX, mouseY }) {
  const { camera } = useThree();

  useFrame(() => {
    camera.position.x += (mouseX.get() * 5 - camera.position.x) * 0.05;
    camera.position.y += (-mouseY.get() * 5 - camera.position.y) * 0.05;
    camera.lookAt(0, 0, 0);
  });
  return null;
}

// --- UI Components ---

function FeatureCard({ icon: Icon, title, description, delay = 0 }) {
  return (
    <motion.div
      initial={{ opacity: 0, y: 30 }}
      whileInView={{ opacity: 1, y: 0 }}
      viewport={{ once: true }}
      transition={{ duration: 0.8, delay }}
      className="relative p-10 rounded-[3rem] bg-white/5 border border-white/10 backdrop-blur-xl group hover:bg-white/10 transition-all duration-500 overflow-hidden"
    >
      <div className="absolute top-0 right-0 p-8 opacity-[0.03] group-hover:opacity-[0.08] transition-opacity">
        <Icon size={120} />
      </div>
      <div className="w-16 h-16 rounded-3xl bg-blue-600/20 flex items-center justify-center mb-8 text-blue-400 group-hover:bg-blue-600 group-hover:text-white transition-all duration-500 shadow-lg shadow-blue-600/20">
        <Icon size={32} />
      </div>
      <h3 className="text-2xl font-black text-white mb-4 tracking-tighter">{title}</h3>
      <p className="text-neutral-400 leading-relaxed font-semibold text-lg">{description}</p>
    </motion.div>
  );
}

// --- Main Page ---

export default function Home() {
  const containerRef = useRef(null);
  const [mounted, setMounted] = useState(false);

  useEffect(() => {
    setMounted(true);
  }, []);

  const containerVariants = {
    hidden: { opacity: 0 },
    visible: {
      opacity: 1,
      transition: {
        staggerChildren: 0.1,
        delayChildren: 0.3
      }
    }
  };

  const itemVariants = {
    hidden: { opacity: 0, y: 20 },
    visible: { opacity: 1, y: 0 }
  };

  return (
    <div className="relative min-h-screen bg-[#050505] text-white selection:bg-blue-600/30 overflow-x-hidden">

      {/* Immersive 3D Experience */}
      {mounted && <Scene3D />}

      <main ref={containerRef} className="relative z-10 w-full">
        {/* Navigation */}
        <nav className="flex items-center justify-between p-10 max-w-7xl mx-auto">
          <motion.div
            initial={{ opacity: 0, scale: 0.8 }}
            animate={{ opacity: 1, scale: 1 }}
            className="text-2xl font-black italic tracking-tighter"
          >
            REGM<span className="text-blue-500">CRAFT</span>
          </motion.div>

          <motion.div className="flex items-center space-x-12">
            <Link href="/dashboard" className="hidden md:block text-xs font-black uppercase tracking-[0.3em] text-neutral-500 hover:text-white transition-colors">Workspace</Link>
            <Link href="/dashboard" className="px-10 py-4 bg-white text-black text-sm font-black italic rounded-full shadow-[0_10px_40px_rgba(255,255,255,0.2)] active:scale-95 transition-transform">
              EXPLORE
            </Link>
          </motion.div>
        </nav>

        {/* Hero Section */}
        <section className="container mx-auto px-6 h-screen flex flex-col items-center justify-center text-center">
          <motion.div
            variants={containerVariants}
            initial="hidden"
            animate="visible"
            className="relative"
          >
            {/* Glassmorphism Title */}
            <motion.div variants={itemVariants} className="mb-6">
              <span className="px-6 py-2 rounded-full bg-blue-600/10 border border-blue-500/20 text-blue-400 text-[10px] font-black uppercase tracking-[0.4em] backdrop-blur-md">
                Protocol: Alpha Release
              </span>
            </motion.div>

            <motion.h1
              variants={itemVariants}
              className="text-8xl md:text-[14rem] font-black italic tracking-tighter leading-none mb-4 mix-blend-screen"
            >
              <span className="block bg-gradient-to-b from-white via-white to-blue-500/50 bg-clip-text text-transparent">
                REGM
              </span>
              <span className="block bg-gradient-to-t from-blue-600 via-indigo-400 to-white bg-clip-text text-transparent -mt-8 md:-mt-16">
                CRAFT
              </span>
            </motion.h1>

            <motion.p
              variants={itemVariants}
              className="max-w-2xl mx-auto text-xl md:text-2xl text-neutral-400 font-medium leading-relaxed tracking-tight"
            >
              Experience the next evolution of game prototyping. Native performance, AI-driven assets, and a <span className="text-white border-b-2 border-blue-600">Unified Web Workspace</span>.
            </motion.p>

            <motion.div variants={itemVariants} className="flex flex-col sm:flex-row items-center justify-center gap-8 mt-16">
              <Link
                href="/dashboard"
                className="group relative px-12 py-6 bg-blue-600 rounded-[2.5rem] text-lg font-black italic text-white shadow-[0_25px_60px_-15px_rgba(37,99,235,0.6)] hover:bg-blue-500 transition-all hover:-translate-y-1 active:scale-95 flex items-center space-x-4"
              >
                <span>LAUNCH CONSOLE</span>
                <ChevronRight size={24} className="group-hover:translate-x-2 transition-transform" />
              </Link>

              <Link
                href="/editor"
                className="px-12 py-6 bg-neutral-900/50 border border-white/10 backdrop-blur-md rounded-[2.5rem] text-lg font-black italic text-neutral-300 hover:text-white transition-all active:scale-95"
              >
                OPEN ENGINE
              </Link>
            </motion.div>
          </motion.div>
        </section>

        {/* Feature Grid with Smooth Reveal */}
        <section className="container mx-auto px-10 pb-40">
          <motion.div
            initial={{ opacity: 0 }}
            whileInView={{ opacity: 1 }}
            className="flex flex-col md:flex-row items-end justify-between mb-20 gap-8"
          >
            <div className="max-w-xl">
              <h2 className="text-5xl md:text-7xl font-black italic tracking-tighter mb-6">ULTRA PERFORMANCE</h2>
              <p className="text-neutral-500 text-xl font-medium">Bypassing browser limitations with our custom C-Engine architecture.</p>
            </div>
            <div className="text-neutral-800 text-9xl font-black italic select-none opacity-20">01</div>
          </motion.div>

          <div className="grid grid-cols-1 md:grid-cols-3 gap-10">
            <FeatureCard
              icon={Cpu}
              title="C-Engine Core"
              description="Low-level memory management for high-speed sprites and massive 2D tilemaps without the overhead."
              delay={0.1}
            />
            <FeatureCard
              icon={Sparkles}
              title="LLM Synthesis"
              description="From natural language to structured JSON logic. Generate world data and behavior trees instantly."
              delay={0.2}
            />
            <FeatureCard
              icon={Layout}
              title="Cloud Workspace"
              description="Real-time multi-platform prototyping. Your engine, your assets, synced everywhere."
              delay={0.3}
            />
          </div>
        </section>

        {/* Footer Glow */}
        <footer className="relative py-20 border-t border-white/5 overflow-hidden">
          <div className="absolute top-0 left-1/2 -translate-x-1/2 w-1/2 h-1/2 bg-blue-600/10 blur-[120px] rounded-full" />
          <div className="container mx-auto px-10 flex flex-col md:flex-row items-center justify-between relative z-10 opacity-40 grayscale group hover:opacity-100 hover:grayscale-0 transition-all duration-1000">
            <div className="text-sm font-black italic tracking-widest mb-10 md:mb-0">© 2026 DEEPMIND AGENTIC RESEARCH</div>
            <div className="flex space-x-12 text-xs font-black uppercase tracking-[0.4em]">
              <Link href="#" className="hover:text-blue-500 transition-colors">Documentation</Link>
              <Link href="#" className="hover:text-blue-500 transition-colors">GitHub</Link>
              <Link href="#" className="hover:text-blue-500 transition-colors">Discord</Link>
            </div>
          </div>
        </footer>
      </main>

      {/* Global CSS for Animations */}
      <style jsx global>{`
        @font-face {
          font-family: 'Outfit';
          src: url('https://fonts.googleapis.com/css2?family=Outfit:wght@100;900&display=swap');
        }
        body {
          background: #050505;
          font-family: 'Outfit', sans-serif;
        }
        .mix-blend-screen {
          mix-blend-mode: screen;
        }
      `}</style>
    </div>
  );
}
