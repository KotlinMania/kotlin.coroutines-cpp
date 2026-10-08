#pragma once

#include <cctype>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>

namespace ast_distance {

// Global per-user config read once from $HOME/.ast_config.
//
// File is JSON, all keys optional. Defaults match the legacy hardcoded
// behavior so a missing file changes nothing:
//
//   {
//     "unlock_parameters": false,    // re-enable --rank/--missing/--summary/--missing-only/--include-stubs
//     "agent_swarm": false           // emit the "Next Commands" task-queue block in NEXT_ACTIONS.md
//   }
//
// The removed redirect_guard key is ignored in existing configurations.
// Normal stdout/stderr capture and pipelines are supported.
//
// The per-project .ast_distance_config.json (source/target/reexport)
// is a separate file and is unaffected by this loader.
struct AstConfig {
    bool unlock_parameters = false;
    bool agent_swarm = false;

    std::string loaded_path;
    bool loaded = false;
};

inline std::string user_ast_config_path() {
    const char* home = std::getenv("HOME");
    if (!home || !*home) return "";
    return std::string(home) + "/.ast_config";
}

namespace ast_config_detail {

inline bool extract_json_bool(const std::string& content,
                              const std::string& key,
                              bool& out) {
    const std::string needle = "\"" + key + "\"";
    size_t pos = content.find(needle);
    if (pos == std::string::npos) return false;
    pos = content.find(':', pos + needle.size());
    if (pos == std::string::npos) return false;
    ++pos;
    while (pos < content.size() &&
           std::isspace(static_cast<unsigned char>(content[pos]))) {
        ++pos;
    }
    if (content.compare(pos, 4, "true") == 0) {
        out = true;
        return true;
    }
    if (content.compare(pos, 5, "false") == 0) {
        out = false;
        return true;
    }
    return false;
}

}  // namespace ast_config_detail

inline AstConfig load_ast_config() {
    AstConfig cfg;
    std::string path = user_ast_config_path();
    if (path.empty()) return cfg;

    std::ifstream in(path);
    if (!in.is_open()) return cfg;

    std::stringstream buf;
    buf << in.rdbuf();
    std::string content = buf.str();

    cfg.loaded = true;
    cfg.loaded_path = path;
    ast_config_detail::extract_json_bool(content, "unlock_parameters", cfg.unlock_parameters);
    ast_config_detail::extract_json_bool(content, "agent_swarm", cfg.agent_swarm);
    return cfg;
}

}  // namespace ast_distance
