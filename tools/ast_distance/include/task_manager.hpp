#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <set>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <algorithm>
#include <regex>
#include <sys/file.h>
#include <unistd.h>
#include <fcntl.h>

namespace ast_distance {

/**
 * RAII file lock for preventing race conditions in concurrent task assignment.
 * Uses flock() for advisory locking on Unix systems.
 */
class FileLock {
public:
    explicit FileLock(const std::string& path) : fd_(-1), locked_(false) {
        lock_path_ = path + ".lock";
        fd_ = open(lock_path_.c_str(), O_CREAT | O_RDWR, 0666);
        if (fd_ >= 0) {
            // Try to acquire exclusive lock (blocking)
            if (flock(fd_, LOCK_EX) == 0) {
                locked_ = true;
            }
        }
    }

    ~FileLock() {
        if (fd_ >= 0) {
            if (locked_) {
                flock(fd_, LOCK_UN);
            }
            close(fd_);
        }
    }

    bool is_locked() const { return locked_; }

    // Prevent copying
    FileLock(const FileLock&) = delete;
    FileLock& operator=(const FileLock&) = delete;

private:
    int fd_;
    bool locked_;
    std::string lock_path_;
};

/**
 * Task status for porting work items.
 */
enum class TaskStatus {
    PENDING,      // Not yet assigned
    ASSIGNED,     // Assigned to an agent
    COMPLETED,    // Successfully ported
    BLOCKED       // Blocked by dependencies
};

/**
 * A single porting task.
 */
struct PortTask {
    std::string source_path;          // Rust source file path
    std::string source_qualified;     // e.g., "core.error"
    std::string target_path;          // Expected Kotlin target path
    std::string target_qualified;     // e.g., "error.CodexError"
    int dependent_count = 0;          // How many files depend on this
    int dependency_count = 0;         // How many files this depends on
    TaskStatus status = TaskStatus::PENDING;
    std::string assigned_to;          // Agent ID
    std::string assigned_at;          // Timestamp
    std::string completed_at;         // Timestamp
    float similarity = 0.0f;          // If partially ported, similarity score
    std::vector<std::string> dependencies;  // Files this depends on
    std::vector<std::string> dependents;    // Files that depend on this
};

/**
 * Task file manager for coordinating swarm agents.
 */
class TaskManager {
public:
    std::string task_file_path;
    std::string agents_md_path;
    std::string source_root;
    std::string target_root;
    std::string source_lang;
    std::string target_lang;
    std::vector<PortTask> tasks;

    TaskManager(const std::string& task_file)
        : task_file_path(task_file) {}

    /**
     * Load tasks from JSON file.
     */
    bool load() {
        std::ifstream file(task_file_path);
        if (!file.is_open()) return false;

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string content = buffer.str();

        // Simple JSON parsing for our format
        tasks.clear();

        // Helper to extract string value
        auto extract_string = [](const std::string& content, const std::string& key) -> std::string {
            std::string pattern = "\"" + key + "\"";
            size_t pos = content.find(pattern);
            if (pos == std::string::npos) return "";
            pos = content.find(':', pos);
            if (pos == std::string::npos) return "";
            pos = content.find('"', pos);
            if (pos == std::string::npos) return "";
            size_t end = content.find('"', pos + 1);
            if (end == std::string::npos) return "";
            return content.substr(pos + 1, end - pos - 1);
        };

        // Helper to extract int value
        auto extract_int = [](const std::string& content, const std::string& key) -> int {
            std::string pattern = "\"" + key + "\"";
            size_t pos = content.find(pattern);
            if (pos == std::string::npos) return 0;
            pos = content.find(':', pos);
            if (pos == std::string::npos) return 0;
            pos++;
            while (pos < content.size() && std::isspace(content[pos])) pos++;
            std::string num;
            while (pos < content.size() && std::isdigit(content[pos])) {
                num += content[pos++];
            }
            return num.empty() ? 0 : std::stoi(num);
        };

        source_root = extract_string(content, "source_root");
        target_root = extract_string(content, "target_root");
        source_lang = extract_string(content, "source_lang");
        target_lang = extract_string(content, "target_lang");
        agents_md_path = extract_string(content, "agents_md");

        // Find tasks array
        size_t tasks_pos = content.find("\"tasks\"");
        if (tasks_pos == std::string::npos) return true;

        // Parse each task object
        size_t pos = tasks_pos;
        while ((pos = content.find("{", pos)) != std::string::npos) {
            size_t end = content.find("}", pos);
            if (end == std::string::npos) break;

            std::string task_str = content.substr(pos, end - pos + 1);

            // Check if this looks like a task (has source_path)
            if (task_str.find("source_path") == std::string::npos) {
                pos = end + 1;
                continue;
            }

            PortTask task;
            task.source_path = extract_string(task_str, "source_path");
            task.source_qualified = extract_string(task_str, "source_qualified");
            task.target_path = extract_string(task_str, "target_path");
            task.dependent_count = extract_int(task_str, "dependent_count");
            task.assigned_to = extract_string(task_str, "assigned_to");
            task.assigned_at = extract_string(task_str, "assigned_at");
            task.completed_at = extract_string(task_str, "completed_at");

            std::string status_str = extract_string(task_str, "status");
            if (status_str == "pending") task.status = TaskStatus::PENDING;
            else if (status_str == "assigned") task.status = TaskStatus::ASSIGNED;
            else if (status_str == "completed") task.status = TaskStatus::COMPLETED;
            else if (status_str == "blocked") task.status = TaskStatus::BLOCKED;

            if (!task.source_path.empty()) {
                tasks.push_back(task);
            }

            pos = end + 1;
        }

        return true;
    }

    /**
     * Save tasks to JSON file.
     */
    bool save() {
        std::ofstream file(task_file_path);
        if (!file.is_open()) return false;

        file << "{\n";
        file << "  \"source_root\": \"" << source_root << "\",\n";
        file << "  \"target_root\": \"" << target_root << "\",\n";
        file << "  \"source_lang\": \"" << source_lang << "\",\n";
        file << "  \"target_lang\": \"" << target_lang << "\",\n";
        file << "  \"agents_md\": \"" << agents_md_path << "\",\n";
        file << "  \"tasks\": [\n";

        for (size_t i = 0; i < tasks.size(); ++i) {
            const auto& t = tasks[i];
            file << "    {\n";
            file << "      \"source_path\": \"" << t.source_path << "\",\n";
            file << "      \"source_qualified\": \"" << t.source_qualified << "\",\n";
            file << "      \"target_path\": \"" << t.target_path << "\",\n";
            file << "      \"dependent_count\": " << t.dependent_count << ",\n";
            file << "      \"status\": \"" << status_to_string(t.status) << "\"";
            if (!t.assigned_to.empty()) {
                file << ",\n      \"assigned_to\": \"" << t.assigned_to << "\"";
            }
            if (!t.assigned_at.empty()) {
                file << ",\n      \"assigned_at\": \"" << t.assigned_at << "\"";
            }
            if (!t.completed_at.empty()) {
                file << ",\n      \"completed_at\": \"" << t.completed_at << "\"";
            }
            file << "\n    }";
            if (i < tasks.size() - 1) file << ",";
            file << "\n";
        }

        file << "  ]\n";
        file << "}\n";

        return true;
    }

    /**
     * Assign the highest-priority pending task to an agent.
     * Returns nullptr if no tasks available.
     *
     * THREAD-SAFE: Uses file locking to prevent race conditions when
     * multiple agents try to grab tasks simultaneously.
     */
    PortTask* assign_next(const std::string& agent_id) {
        // Acquire exclusive lock on the task file
        FileLock lock(task_file_path);
        if (!lock.is_locked()) {
            std::cerr << "Warning: Could not acquire lock on task file\n";
            return nullptr;
        }

        // Reload fresh data from disk (another agent may have modified it)
        if (!load()) {
            std::cerr << "Warning: Could not reload task file\n";
            return nullptr;
        }

        // Sort by dependent_count descending (highest priority first)
        std::vector<PortTask*> pending;
        for (auto& t : tasks) {
            if (t.status == TaskStatus::PENDING) {
                pending.push_back(&t);
            }
        }

        if (pending.empty()) return nullptr;

        std::sort(pending.begin(), pending.end(),
            [](const PortTask* a, const PortTask* b) {
                return a->dependent_count > b->dependent_count;
            });

        PortTask* task = pending[0];
        task->status = TaskStatus::ASSIGNED;
        task->assigned_to = agent_id;
        task->assigned_at = current_timestamp();

        // Save immediately while still holding lock
        if (!save()) {
            std::cerr << "Warning: Could not save task file after assignment\n";
            // Revert the in-memory change
            task->status = TaskStatus::PENDING;
            task->assigned_to.clear();
            task->assigned_at.clear();
            return nullptr;
        }

        return task;
    }

    /**
     * Mark a task as completed.
     * THREAD-SAFE: Uses file locking.
     */
    bool complete_task(const std::string& source_qualified) {
        FileLock lock(task_file_path);
        if (!lock.is_locked()) {
            std::cerr << "Warning: Could not acquire lock for complete_task\n";
            return false;
        }

        // Reload fresh data
        if (!load()) return false;

        for (auto& t : tasks) {
            if (t.source_qualified == source_qualified) {
                t.status = TaskStatus::COMPLETED;
                t.completed_at = current_timestamp();
                return save();
            }
        }
        return false;
    }

    /**
     * Release an assigned task back to pending.
     * THREAD-SAFE: Uses file locking.
     */
    bool release_task(const std::string& source_qualified) {
        FileLock lock(task_file_path);
        if (!lock.is_locked()) {
            std::cerr << "Warning: Could not acquire lock for release_task\n";
            return false;
        }

        // Reload fresh data
        if (!load()) return false;

        for (auto& t : tasks) {
            if (t.source_qualified == source_qualified && t.status == TaskStatus::ASSIGNED) {
                t.status = TaskStatus::PENDING;
                t.assigned_to.clear();
                t.assigned_at.clear();
                return save();
            }
        }
        return false;
    }

    /**
     * Get task statistics.
     */
    void get_stats(int& pending, int& assigned, int& completed, int& blocked) const {
        pending = assigned = completed = blocked = 0;
        for (const auto& t : tasks) {
            switch (t.status) {
                case TaskStatus::PENDING: ++pending; break;
                case TaskStatus::ASSIGNED: ++assigned; break;
                case TaskStatus::COMPLETED: ++completed; break;
                case TaskStatus::BLOCKED: ++blocked; break;
            }
        }
    }

    /**
     * Read AGENTS.md content if it exists.
     */
    std::string read_agents_md() const {
        if (agents_md_path.empty()) return "";
        std::ifstream file(agents_md_path);
        if (!file.is_open()) return "";
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

	    /**
	     * Print task assignment details for an agent.
	     */
	    void print_assignment(const PortTask& task, int agent_number) const {
	        std::cout << "=== TASK ASSIGNMENT ===\n\n";
	        std::cout << "You are agent #" << agent_number << "\n";
	        std::cout << "Reminder: all ast_distance commands require: --agent "
	                  << agent_number << "\n\n";

	        auto is_test_task = [&]() -> bool {
	            return task.source_path.rfind("tests/", 0) == 0;
	        };

	        auto effective_target_root = [&]() -> std::string {
	            // Kotlin Multiplatform convention: tests live under src/commonTest, not src/commonMain.
	            // Task files typically only store a single target_root; adjust on demand for test tasks.
	            if (target_lang != "kotlin" || !is_test_task()) {
	                return target_root;
	            }
	            std::string root = target_root;
	            auto replace_all = [&](const std::string& from, const std::string& to) {
	                size_t pos = 0;
	                while ((pos = root.find(from, pos)) != std::string::npos) {
	                    root.replace(pos, from.size(), to);
	                    pos += to.size();
	                }
	            };
	            replace_all("/src/commonMain/", "/src/commonTest/");
	            replace_all("src/commonMain/", "src/commonTest/");
	            return root;
	        };

	        std::cout << "Source File:\n";
	        std::cout << "  Path:      " << source_root << "/" << task.source_path << "\n";
	        std::cout << "  Qualified: " << task.source_qualified << "\n";
        std::cout << "  Dependents: " << task.dependent_count << " files depend on this\n\n";

        std::cout << "Target File:\n";
        std::cout << "  Path:      " << effective_target_root() << "/" << task.target_path << "\n";
        std::string port_lint_path = task.source_path;
        auto ends_with = [](const std::string& s, const std::string& suffix) -> bool {
            return s.size() >= suffix.size() &&
                   s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
        };
        if (ends_with(source_root, "/src") || ends_with(source_root, "/src/") ||
            ends_with(source_root, "\\src") || ends_with(source_root, "\\src\\")) {
            if (port_lint_path.rfind("src/", 0) != 0 && port_lint_path.rfind("src\\", 0) != 0) {
                port_lint_path = "src/" + port_lint_path;
            }
        }
        std::cout << "  Add header: " << port_lint_comment_prefix(target_lang)
                  << " port-lint: " << (is_test_task() ? "tests " : "source ")
                  << port_lint_path << "\n\n";

        std::cout << "Priority: " << task.dependent_count << " (higher = more critical)\n\n";

        // Read and display AGENTS.md guidelines
        std::string agents_content = read_agents_md();
        if (!agents_content.empty()) {
            std::cout << "=== PORTING GUIDELINES (from AGENTS.md) ===\n\n";
            std::cout << agents_content << "\n";
        }

        std::cout << "=== INSTRUCTIONS ===\n\n";
        std::cout << "1. Read the source file thoroughly\n";
        std::cout << "2. Create the target file at the target path\n";
        std::cout << "3. Add the port-lint header as the first line\n";
	        std::cout << "4. Transliterate the source code to idiomatic target language\n";
	        std::cout << "5. Match documentation comments from the source\n";
	        std::cout << "6. Run: ast_distance --agent " << agent_number << " <source> " << source_lang
	                  << " <target> " << target_lang << "\n";
	        std::cout << "   to verify similarity (aim for >0.85)\n";
	        std::cout << "7. When complete, run: ast_distance --agent " << agent_number
	                  << " --complete " << task_file_path << " " << task.source_qualified << "\n\n";
	    }

private:
    static const char* port_lint_comment_prefix(const std::string& lang) {
        // Use a visually-distinct prefix for Python to avoid confusion with ordinary comments.
        if (lang == "python") return "##";
        return "//";
    }

    static std::string status_to_string(TaskStatus s) {
        switch (s) {
            case TaskStatus::PENDING: return "pending";
            case TaskStatus::ASSIGNED: return "assigned";
            case TaskStatus::COMPLETED: return "completed";
            case TaskStatus::BLOCKED: return "blocked";
        }
        return "unknown";
    }

    static std::string current_timestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%dT%H:%M:%S");
        return ss.str();
    }
};

} // namespace ast_distance
