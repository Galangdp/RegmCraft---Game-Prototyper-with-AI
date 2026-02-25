"use client";
import { useState, useEffect, useRef } from "react";
import { toast } from "sonner";
import { motion, AnimatePresence } from "framer-motion";
import { LayoutGrid, Clock, Trash2, Search, Plus, MoreVertical, ExternalLink, RotateCcw, Edit2, Copy, Download, Check, LogOut, User } from "lucide-react";
import Link from "next/link";

import { useRouter } from "next/navigation";
import { supabase } from "@/lib/supabase";

export function Sidebar({ activeTab, setActiveTab, user }) {
    const router = useRouter();
    const menuItems = [
        { id: "all", label: "All Projects", icon: LayoutGrid },
        { id: "recent", label: "Recent", icon: Clock },
        { id: "trash", label: "Trash", icon: Trash2 },
    ];

    const handleLogout = async () => {
        try {
            await supabase.auth.signOut();
            router.push("/auth");
            router.refresh();
        } catch (error) {
            console.error("Logout error:", error);
            // Fallback redirect
            window.location.href = "/auth";
        }
    };

    return (
        <aside className="w-64 border-r border-neutral-800 bg-neutral-900/50 flex flex-col h-screen font-sans">
            <div className="p-6">
                <h2 className="text-xl font-black italic bg-gradient-to-r from-blue-400 to-indigo-500 bg-clip-text text-transparent">
                    RegmCraft
                </h2>
            </div>

            <nav className="flex-1 px-4 space-y-2">
                {menuItems.map((item) => {
                    const Icon = item.icon;
                    const isActive = activeTab === item.id;
                    return (
                        <button
                            key={item.id}
                            onClick={() => setActiveTab(item.id)}
                            className={`w-full flex items-center space-x-3 px-4 py-3 rounded-xl transition-all duration-300 group relative ${isActive
                                ? "bg-blue-600/10 text-blue-400"
                                : "text-neutral-400 hover:text-white hover:bg-neutral-800"
                                }`}
                        >
                            {isActive && (
                                <motion.div
                                    layoutId="sidebar-active"
                                    className="absolute left-0 w-1 h-6 bg-blue-500 rounded-r-full"
                                    transition={{ type: "spring", stiffness: 300, damping: 30 }}
                                />
                            )}
                            <Icon size={20} className={isActive ? "text-blue-400" : "group-hover:scale-110 transition-transform"} />
                            <span className="font-medium">{item.label}</span>
                        </button>
                    );
                })}
            </nav>

            <div className="p-4 border-t border-neutral-800 space-y-4">
                {user && (
                    <div className="bg-neutral-800/50 rounded-2xl p-4 flex items-center space-x-3">
                        <div className="w-8 h-8 rounded-full bg-blue-600 flex items-center justify-center text-xs font-bold text-white uppercase">
                            {user.email?.[0] || <User size={14} />}
                        </div>
                        <div className="overflow-hidden">
                            <p className="text-[10px] text-neutral-500 uppercase font-bold tracking-wider">Account</p>
                            <p className="text-xs text-neutral-300 truncate font-medium">{user.email}</p>
                        </div>
                    </div>
                )}

                <button
                    onClick={handleLogout}
                    className="w-full flex items-center space-x-3 px-4 py-3 rounded-xl text-red-400 hover:bg-red-500/10 transition-all duration-300 group"
                >
                    <LogOut size={20} className="group-hover:translate-x-1 transition-transform" />
                    <span className="font-semibold">Logout</span>
                </button>

                <div className="bg-neutral-800/30 rounded-2xl p-4">
                    <p className="text-[10px] text-neutral-500 mb-1 font-bold uppercase tracking-wider">Storage</p>
                    <div className="h-1.5 w-full bg-neutral-700 rounded-full overflow-hidden">
                        <div className="h-full bg-blue-500 w-1/3 shadow-[0_0_10px_rgba(59,130,246,0.5)]" />
                    </div>
                    <p className="text-[9px] text-neutral-600 mt-2 text-center font-medium">Cloud Database Connected</p>
                </div>
            </div>
        </aside>
    );
}

export function Header({ searchQuery, setSearchQuery, onNewProject }) {
    return (
        <header className="h-20 border-b border-neutral-800 flex items-center justify-between px-8 bg-neutral-950/50 backdrop-blur-md sticky top-0 z-10">
            <div className="flex-1 max-w-xl">
                <div className="relative group">
                    <Search className="absolute left-4 top-1/2 -translate-y-1/2 text-neutral-500 group-focus-within:text-blue-400 transition-colors" size={18} />
                    <input
                        type="text"
                        value={searchQuery}
                        onChange={(e) => setSearchQuery(e.target.value)}
                        placeholder="Search projects..."
                        className="w-full bg-neutral-900/50 border border-neutral-800 rounded-2xl py-2.5 pl-12 pr-4 text-sm focus:outline-none focus:ring-2 focus:ring-blue-500/20 focus:border-blue-500/50 transition-all text-white"
                    />
                </div>
            </div>

            <button
                onClick={onNewProject}
                className="ml-6 flex items-center space-x-2 bg-blue-600 hover:bg-blue-500 text-white px-5 py-2.5 rounded-2xl font-bold transition-all hover:shadow-[0_0_20px_rgba(37,99,235,0.3)] active:scale-95"
            >
                <Plus size={20} />
                <span>New Project</span>
            </button>
        </header>
    );
}

const MenuOption = ({ icon: Icon, label, onClick, variant = "default" }) => (
    <button
        onClick={(e) => {
            e.stopPropagation();
            onClick();
        }}
        className={`w-full flex items-center space-x-3 px-3 py-2.5 rounded-lg text-sm transition-all ${variant === "danger"
            ? "text-red-400 hover:bg-red-500/10"
            : "text-neutral-300 hover:bg-white/10"
            }`}
    >
        <Icon size={16} />
        <span className="font-medium">{label}</span>
    </button>
);

export function ContextMenu({ onClose, onRename, onDuplicate, onCopyId, onExport, onDelete }) {
    const menuRef = useRef(null);

    useEffect(() => {
        const handleClickOutside = (event) => {
            if (menuRef.current && !menuRef.current.contains(event.target)) {
                onClose();
            }
        };
        document.addEventListener("mousedown", handleClickOutside);
        return () => document.removeEventListener("mousedown", handleClickOutside);
    }, [onClose]);

    return (
        <motion.div
            ref={menuRef}
            initial={{ opacity: 0 }}
            animate={{ opacity: 1 }}
            exit={{ opacity: 0 }}
            transition={{ duration: 0.05 }}
            className="absolute right-0 top-full mt-2 w-64 z-[999] bg-[#121212] border border-white/10 p-2 rounded-2xl shadow-[0_20px_60px_rgba(0,0,0,0.95)]"
        >
            <div className="space-y-1">
                <MenuOption icon={Edit2} label="Rename" onClick={onRename} />
                <MenuOption icon={Copy} label="Duplicate" onClick={onDuplicate} />
                <MenuOption icon={Check} label="Copy Project ID" onClick={onCopyId} />
                <MenuOption icon={Download} label="Export as JSON" onClick={onExport} />
                <div className="h-px bg-white/10 my-2 mx-1" />
                <MenuOption icon={Trash2} label="Move to Trash" onClick={onDelete} variant="danger" />
            </div>
        </motion.div>
    );
}

export function RenameModal({ isOpen, currentName, onClose, onRename }) {
    const [name, setName] = useState(currentName);
    const [isLoading, setIsLoading] = useState(false);

    const handleSubmit = async () => {
        if (!name || name === currentName) {
            onClose();
            return;
        }
        setIsLoading(true);
        await onRename(name);
        setIsLoading(false);
        onClose();
    };

    if (!isOpen) return null;

    return (
        <div className="fixed inset-0 z-[10000] flex items-center justify-center p-4 bg-black/80 backdrop-blur-md">
            <motion.div
                initial={{ opacity: 0, scale: 0.9, y: 20 }}
                animate={{ opacity: 1, scale: 1, y: 0 }}
                className="bg-neutral-900 border border-white/5 w-full max-w-sm rounded-[2.5rem] p-10 shadow-3xl text-white"
            >
                <h2 className="text-2xl font-bold mb-2 tracking-tight">Rename Workspace</h2>
                <p className="text-neutral-400 text-sm mb-8">Choose a clear name for your project.</p>

                <input
                    autoFocus
                    disabled={isLoading}
                    type="text"
                    value={name}
                    onChange={(e) => setName(e.target.value)}
                    className="w-full bg-black/40 border border-white/10 rounded-2xl py-4 px-6 mb-10 focus:outline-none focus:ring-2 focus:ring-blue-500/50 text-white font-medium disabled:opacity-50"
                    onKeyDown={(e) => e.key === "Enter" && handleSubmit()}
                />

                <div className="flex space-x-4">
                    <button
                        onClick={onClose}
                        disabled={isLoading}
                        className="flex-1 py-4 rounded-2xl bg-neutral-800 hover:bg-neutral-700 font-bold transition-all text-sm tracking-wider disabled:opacity-50"
                    >
                        Cancel
                    </button>
                    <button
                        onClick={handleSubmit}
                        disabled={!name || isLoading}
                        className="flex-1 py-4 rounded-2xl bg-blue-600 hover:bg-blue-500 font-bold transition-all shadow-xl shadow-blue-600/20 text-sm tracking-wider flex items-center justify-center space-x-2 disabled:opacity-50"
                    >
                        {isLoading ? (
                            <div className="w-4 h-4 border-2 border-white/30 border-t-white rounded-full animate-spin" />
                        ) : (
                            <span>Rename</span>
                        )}
                    </button>
                </div>
            </motion.div>
        </div>
    );
}

export function ProjectCard({ project, onDelete, onRestore, onDeletePermanently, onRename, onDuplicate }) {
    const [isMenuOpen, setIsMenuOpen] = useState(false);
    const [isRenameOpen, setIsRenameOpen] = useState(false);

    const toggleMenu = (e) => {
        e.stopPropagation();
        setIsMenuOpen(!isMenuOpen);
    };

    const handleCopyId = () => {
        navigator.clipboard.writeText(project.id);
        toast.success("Project ID copied to clipboard!");
        setIsMenuOpen(false);
    };

    const handleExport = () => {
        const dataStr = "data:text/json;charset=utf-8," + encodeURIComponent(JSON.stringify(project, null, 2));
        const downloadAnchorNode = document.createElement('a');
        downloadAnchorNode.setAttribute("href", dataStr);
        downloadAnchorNode.setAttribute("download", `${project.name.replace(/\s+/g, '_')}.json`);
        document.body.appendChild(downloadAnchorNode);
        downloadAnchorNode.click();
        downloadAnchorNode.remove();
        toast.success("Project data exported safely!");
        setIsMenuOpen(false);
    };

    return (
        <>
            <motion.div
                initial={{ opacity: 0, scale: 0.9 }}
                animate={{ opacity: 1, scale: 1 }}
                whileHover={{ y: -8 }}
                style={{ zIndex: isMenuOpen ? 50 : 1 }}
                className="bg-neutral-900 border border-white/5 rounded-[2.5rem] group hover:border-white/10 transition-all relative overflow-visible"
            >
                {/* Visual Focus Image Area */}
                <div className="h-48 relative overflow-hidden flex items-center justify-center p-6 rounded-t-[2.5rem] border-b border-white/5 bg-gradient-to-br from-neutral-800/10 to-transparent">
                    {project.preview ? (
                        <img src={project.preview} alt={project.name} className="object-cover w-full h-full transform group-hover:scale-105 transition-transform duration-1000" />
                    ) : (
                        <div className="text-5xl text-white font-black italic select-none opacity-[0.02] group-hover:opacity-[0.05] transition-opacity tracking-tight transform group-hover:scale-110 transition-transform duration-1000">
                            REGM
                        </div>
                    )}
                    {/* NO MIDDLE OVERLAYS - CLEAN LAYOUT */}
                </div>

                <div className="p-8">
                    <div className="flex items-start justify-between">
                        <div className="overflow-hidden pr-4 flex-1">
                            <h3 className="font-bold text-neutral-100 group-hover:text-blue-500 transition-colors truncate text-xl tracking-tight">
                                {project.name}
                            </h3>
                            <p className="text-[10px] uppercase font-black tracking-[0.2em] text-neutral-500 mt-2.5 opacity-40">
                                {project.isDeleted ? "Status: Archived" : `Updated: ${new Date(project.lastModified).toLocaleDateString()}`}
                            </p>
                        </div>

                        {/* STRICT ANCHORING: DEDICATED RELATIVE WRAPPER */}
                        <div className="relative">
                            <motion.button
                                whileHover={{ scale: 1.1 }}
                                whileTap={{ scale: 0.9 }}
                                onClick={toggleMenu}
                                className={`p-2.5 rounded-2xl transition-all ${isMenuOpen ? 'bg-blue-600 text-white shadow-xl shadow-blue-600/20' : 'text-neutral-500 hover:text-white hover:bg-white/10'}`}
                            >
                                <MoreVertical size={20} />
                            </motion.button>

                            <AnimatePresence>
                                {isMenuOpen && (
                                    <ContextMenu
                                        onClose={() => setIsMenuOpen(false)}
                                        onRename={() => { setIsRenameOpen(true); setIsMenuOpen(false); }}
                                        onDuplicate={() => { onDuplicate(project.id); setIsMenuOpen(false); }}
                                        onCopyId={handleCopyId}
                                        onExport={handleExport}
                                        onDelete={() => { onDelete(project.id); setIsMenuOpen(false); }}
                                    />
                                )}
                            </AnimatePresence>
                        </div>
                    </div>

                    <div className="mt-10">
                        {!project.isDeleted ? (
                            <Link
                                href={`/editor/${project.id}`}
                                className="w-full flex items-center justify-center space-x-3 py-4 rounded-[1.5rem] bg-neutral-800/40 border border-white/5 hover:bg-blue-600 hover:border-blue-500 hover:text-white transition-all font-bold text-sm tracking-wide text-neutral-400 group-context-action"
                            >
                                <span>Open Project</span>
                                <ExternalLink size={16} className="opacity-40" />
                            </Link>
                        ) : (
                            <button
                                onClick={() => onRestore(project.id)}
                                className="w-full flex items-center justify-center space-x-3 py-4 rounded-[1.5rem] bg-blue-600/10 border border-blue-500/20 hover:bg-blue-600 text-blue-400 hover:text-white transition-all font-bold text-sm"
                            >
                                <RotateCcw size={18} />
                                <span>Restore Flow</span>
                            </button>
                        )}
                    </div>
                </div>
            </motion.div>

            <RenameModal
                isOpen={isRenameOpen}
                currentName={project.name}
                onClose={() => setIsRenameOpen(false)}
                onRename={(newName) => {
                    onRename(project.id, newName);
                    setIsRenameOpen(false);
                }}
            />
        </>
    );
}

export function NewProjectModal({ isOpen, onClose, onCreate }) {
    const [name, setName] = useState("");
    const [isLoading, setIsLoading] = useState(false);

    const handleSubmit = async () => {
        if (!name) return;
        setIsLoading(true);
        await onCreate(name);
        setIsLoading(false);
        // Page handler will handle closing modal and notifications
    };

    if (!isOpen) return null;

    return (
        <div className="fixed inset-0 z-50 flex items-center justify-center p-4 bg-black/80 backdrop-blur-sm">
            <motion.div
                initial={{ opacity: 0, scale: 0.9, y: 20 }}
                animate={{ opacity: 1, scale: 1, y: 0 }}
                exit={{ opacity: 0, scale: 0.9, y: 20 }}
                className="bg-neutral-900 border border-neutral-800 w-full max-w-md rounded-3xl p-8 shadow-2xl text-white"
            >
                <h2 className="text-2xl font-bold mb-2">Create Project</h2>
                <p className="text-neutral-400 text-sm mb-6">Give your masterpiece a name to get started.</p>

                <input
                    autoFocus
                    disabled={isLoading}
                    type="text"
                    value={name}
                    onChange={(e) => setName(e.target.value)}
                    placeholder="e.g. Dungeon Crawler"
                    className="w-full bg-neutral-800 border border-neutral-700 rounded-2xl py-3 px-4 mb-8 focus:outline-none focus:ring-2 focus:ring-blue-500/50 text-white disabled:opacity-50"
                    onKeyDown={(e) => e.key === "Enter" && handleSubmit()}
                />

                <div className="flex space-x-3">
                    <button
                        onClick={onClose}
                        disabled={isLoading}
                        className="flex-1 py-3 rounded-2xl bg-neutral-800 hover:bg-neutral-700 font-semibold transition-colors disabled:opacity-50"
                    >
                        Cancel
                    </button>
                    <button
                        onClick={handleSubmit}
                        disabled={!name || isLoading}
                        className="flex-1 py-3 rounded-2xl bg-blue-600 hover:bg-blue-500 disabled:opacity-50 disabled:cursor-not-allowed font-bold transition-all shadow-lg shadow-blue-600/20 flex items-center justify-center space-x-2"
                    >
                        {isLoading ? (
                            <div className="w-4 h-4 border-2 border-white/30 border-t-white rounded-full animate-spin" />
                        ) : (
                            <span>Create</span>
                        )}
                    </button>
                </div>
            </motion.div>
        </div>
    );
}
