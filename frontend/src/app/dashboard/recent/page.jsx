"use client";
import { useState, useMemo, useEffect } from "react";
import { toast } from "sonner";
import { useProjects } from "@/hooks/useProjects";
import { useAuth } from "@/hooks/useAuth";
import { Sidebar, Header, ProjectCard, NewProjectModal } from "@/components/dashboard/DashboardComponents";
import { useRouter } from "next/navigation";
import { AnimatePresence, motion } from "framer-motion";

export default function RecentPage() {
    const { user, loading: authLoading, signOut } = useAuth();
    const router = useRouter();
    const [isMounted, setIsMounted] = useState(false);
    const {
        projects,
        loading: projectsLoading,
        deleteProject,
        restoreProject,
        deletePermanently,
        addProject,
        renameProject,
        duplicateProject
    } = useProjects();

    const [searchQuery, setSearchQuery] = useState("");
    const [isModalOpen, setIsModalOpen] = useState(false);

    useEffect(() => {
        if (!authLoading && !user) {
            router.push("/auth");
        }
    }, [user, authLoading, router]);

    useEffect(() => {
        setIsMounted(true);
    }, []);

    const handleCreateProject = async (name) => {
        const promise = addProject(name);
        toast.promise(promise, {
            loading: 'Creating your project...',
            success: (data) => {
                setIsModalOpen(false);
                return `Project "${name}" created successfully!`;
            },
            error: 'Failed to create project.'
        });
    };

    const handleDelete = async (id) => {
        const promise = deleteProject(id);
        toast.promise(promise, {
            loading: 'Moving to trash...',
            success: 'Project moved to trash.',
            error: 'Failed to delete project.'
        });
    };

    const handleRename = async (id, newName) => {
        const promise = renameProject(id, newName);
        toast.promise(promise, {
            loading: 'Renaming project...',
            success: 'Project renamed successfully.',
            error: 'Failed to rename project.'
        });
    };

    const handleDuplicate = async (id) => {
        const promise = duplicateProject(id);
        toast.promise(promise, {
            loading: 'Duplicating project...',
            success: 'Project duplicated successfully.',
            error: 'Failed to duplicate project.'
        });
    };

    const handleRestore = async (id) => {
        const promise = restoreProject(id);
        toast.promise(promise, {
            loading: 'Restoring project...',
            success: 'Project restored successfully.',
            error: 'Failed to restore project.'
        });
    };

    const handleDeletePermanently = async (id) => {
        const promise = deletePermanently(id);
        toast.promise(promise, {
            loading: 'Deleting permanently...',
            success: 'Project deleted permanently.',
            error: 'Failed to delete project.'
        });
    };

    const filteredProjects = useMemo(() => {
        if (!projects) return [];

        return projects
            .filter((project) => {
                // Ignore deleted projects in Recent view
                if (project.isDeleted) return false;

                // Recent = last 24 hours
                const ONE_DAY_MS = 24 * 60 * 60 * 1000;
                const diff = new Date() - new Date(project.lastModified);
                if (diff > ONE_DAY_MS) return false;

                // Search Filter
                if (searchQuery && !project.name.toLowerCase().includes(searchQuery.toLowerCase())) {
                    return false;
                }
                return true;
            })
            .sort((a, b) => new Date(b.lastModified) - new Date(a.lastModified))
            .slice(0, 6); // Limit to top 6
    }, [projects, searchQuery]);

    if (!isMounted) return null;

    // Safety Check & Hydration Fix
    if (authLoading || projectsLoading || !user) {
        return (
            <div className="flex h-screen items-center justify-center bg-neutral-950 text-white font-sans">
                <div className="flex flex-col items-center space-y-4">
                    <div className="w-12 h-12 border-4 border-blue-600 border-t-transparent rounded-full animate-spin" />
                    <p className="text-neutral-500 font-medium tracking-tight">
                        {!user && !authLoading ? "Redirecting to Login..." : "Accessing Recent Drafts..."}
                    </p>
                </div>
            </div>
        );
    }

    const handleSignOut = async () => {
        await signOut();
        router.push("/auth");
    };

    return (
        <div className="flex h-screen bg-neutral-950 text-white overflow-hidden font-sans">
            {/* Note: In a dedicated route, Sidebar might need navigation logic, 
                but for now we keep consistency with the Dashboard components */}
            <Sidebar
                activeTab="recent"
                setActiveTab={(tab) => {
                    if (tab === 'all') router.push('/dashboard');
                    if (tab === 'trash') router.push('/dashboard?tab=trash');
                }}
                user={user}
                onSignOut={handleSignOut}
            />

            <div className="flex-1 flex flex-col overflow-hidden">
                <Header
                    searchQuery={searchQuery}
                    setSearchQuery={setSearchQuery}
                    onNewProject={() => setIsModalOpen(true)}
                />

                <main className="flex-1 overflow-y-auto p-8 custom-scrollbar">
                    <header className="mb-10">
                        <h1 className="text-3xl font-bold flex items-center space-x-3 tracking-tight">
                            <span>Recent Drafts</span>
                            <span className="text-sm font-normal text-neutral-500 bg-neutral-900 px-3 py-1 rounded-full ml-4">
                                {filteredProjects.length}
                            </span>
                        </h1>
                    </header>

                    {filteredProjects.length === 0 ? (
                        <motion.div
                            initial={{ opacity: 0, y: 20 }}
                            animate={{ opacity: 1, y: 0 }}
                            className="h-[60vh] flex flex-col items-center justify-center text-center bg-neutral-900/20 rounded-3xl border border-dashed border-neutral-800"
                        >
                            <div className="text-6xl mb-4 grayscale opacity-20">🕒</div>
                            <h3 className="text-xl font-semibold text-neutral-400">
                                {searchQuery ? "No matches in recent drafts" : "No recent activity"}
                            </h3>
                            <p className="text-neutral-500 mt-2 max-w-xs px-4">
                                Any project modified in the last 24 hours will appear here.
                            </p>
                        </motion.div>
                    ) : (
                        <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 xl:grid-cols-4 gap-6">
                            <AnimatePresence mode="popLayout">
                                {filteredProjects.map((project) => (
                                    <ProjectCard
                                        key={project.id}
                                        project={project}
                                        onDelete={handleDelete}
                                        onRestore={handleRestore}
                                        onDeletePermanently={handleDeletePermanently}
                                        onRename={handleRename}
                                        onDuplicate={handleDuplicate}
                                    />
                                ))}
                            </AnimatePresence>
                        </div>
                    )}
                </main>
            </div>

            <AnimatePresence>
                {isModalOpen && (
                    <NewProjectModal
                        isOpen={isModalOpen}
                        onClose={() => setIsModalOpen(false)}
                        onCreate={handleCreateProject}
                    />
                )}
            </AnimatePresence>

            <style jsx global>{`
                .custom-scrollbar::-webkit-scrollbar { width: 8px; }
                .custom-scrollbar::-webkit-scrollbar-track { background: transparent; }
                .custom-scrollbar::-webkit-scrollbar-thumb { background: #262626; border-radius: 10px; }
                .custom-scrollbar::-webkit-scrollbar-thumb:hover { background: #404040; }
            `}</style>
        </div>
    );
}
