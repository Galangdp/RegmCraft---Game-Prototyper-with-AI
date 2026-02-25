"use client";
import { useState, useEffect } from "react";
import { supabase } from "@/lib/supabase";
import { useAuth } from "./useAuth";

export function useProjects() {
    const { user } = useAuth();
    const [projects, setProjects] = useState([]);
    const [loading, setLoading] = useState(true);

    const fetchProjects = async (showLoading = true) => {
        if (!user) return;
        if (showLoading) setLoading(true);
        const { data, error } = await supabase
            .from("projects")
            .select("*")
            .eq("user_id", user.id) // STRICT FILTERING
            .order("last_modified", { ascending: false });

        if (error) {
            console.error("Failed to fetch projects:", error);
        } else {
            setProjects(data.map(p => ({
                ...p,
                createdAt: p.last_modified,
                isDeleted: p.is_deleted,
            })));
        }
        setLoading(false);
    };

    // Load projects from Supabase
    useEffect(() => {
        if (!user) {
            setLoading(false);
            return;
        }

        fetchProjects();

        // Optional: Real-time subscription
        const subscription = supabase
            .channel("projects_changes")
            .on("postgres_changes", { event: "*", schema: "public", table: "projects" }, () => fetchProjects(false))
            .subscribe();

        return () => {
            supabase.removeChannel(subscription);
        };
    }, [user]);

    const addProject = async (name) => {
        const { data: { user: currentUser }, error: authError } = await supabase.auth.getUser();

        if (authError || !currentUser) return null;

        const newProject = {
            name,
            data: {},
            user_id: currentUser.id,
            is_deleted: false,
        };

        const { data, error } = await supabase
            .from("projects")
            .insert([newProject])
            .select()
            .single();

        if (error) {
            console.error("Insert error:", error);
            return null;
        }

        // Trigger manual refresh
        await fetchProjects(false);
        return data;
    };

    const deleteProject = async (id) => {
        // Optimistic Update
        setProjects(prev => prev.map(p => p.id === id ? { ...p, isDeleted: true } : p));

        const { error } = await supabase
            .from("projects")
            .update({ is_deleted: true, last_modified: new Date().toISOString() })
            .eq("id", id);

        if (error) {
            console.error("Soft delete error:", error);
            await fetchProjects(false); // Revert on error
        }
    };

    const restoreProject = async (id) => {
        // Optimistic Update
        setProjects(prev => prev.map(p => p.id === id ? { ...p, isDeleted: false } : p));

        const { error } = await supabase
            .from("projects")
            .update({ is_deleted: false, last_modified: new Date().toISOString() })
            .eq("id", id);

        if (error) {
            console.error("Restore error:", error);
            await fetchProjects(false);
        }
    };

    const deletePermanently = async (id) => {
        // Optimistic Update
        setProjects(prev => prev.filter(p => p.id !== id));

        const { error } = await supabase
            .from("projects")
            .delete()
            .eq("id", id);

        if (error) {
            console.error("Delete error:", error);
            await fetchProjects(false);
        }
    };

    const renameProject = async (id, newName) => {
        // Optimistic Update
        setProjects(prev => prev.map(p => p.id === id ? { ...p, name: newName } : p));

        const { error } = await supabase
            .from("projects")
            .update({ name: newName, last_modified: new Date().toISOString() })
            .eq("id", id);

        if (error) {
            console.error("Rename error:", error);
            await fetchProjects(false);
        }
    };

    const duplicateProject = async (id) => {
        const { data: { user: currentUser } } = await supabase.auth.getUser();
        if (!currentUser) return;

        const original = projects.find((p) => p.id === id);
        if (!original) return;

        const { id: _, last_modified: __, is_deleted: ___, ...rest } = original;
        const newProject = {
            ...rest,
            name: `${original.name} (Copy)`,
            user_id: currentUser.id,
            is_deleted: false,
        };

        const { error } = await supabase.from("projects").insert([newProject]);
        if (error) {
            console.error("Duplicate error:", error);
        } else {
            await fetchProjects(false);
        }
    };

    return {
        projects,
        loading,
        addProject,
        deleteProject,
        restoreProject,
        deletePermanently,
        renameProject,
        duplicateProject,
        refreshProjects: fetchProjects
    };
}
