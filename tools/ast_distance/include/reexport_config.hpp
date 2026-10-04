#pragma once

#include <fnmatch.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace ast_distance {

/**
 * Optional config loaded from .ast_distance_config.json.
 *
 * The same file owns the local source/target mapping and the reporting
 * whitelist. Source checkouts normally live under tmp/ and are not committed,
 * so keeping this as a dotfile makes the local dependency explicit without
 * implying that the source tree itself belongs to the port repository.
 */
struct ConfigEndpoint {
    std::string path;
    std::string lang;
};

struct ReexportConfig {
    std::string type;
    std::string name;
    ConfigEndpoint source;
    ConfigEndpoint target;
    std::vector<std::string> patterns;
    std::string config_path;
    bool loaded = false;
    bool had_parse_error = false;

    bool matches(const std::string& relative_path) const {
        if (!loaded || patterns.empty()) return false;
        std::filesystem::path rel(relative_path);
        std::string basename = rel.filename().string();
        for (const auto& pat : patterns) {
            int flags = 0;
            const std::string* target = &basename;
            if (pat.find('/') != std::string::npos) {
                flags = FNM_PATHNAME;
                target = &relative_path;
            }
            if (::fnmatch(pat.c_str(), target->c_str(), flags) == 0) {
                return true;
            }
        }
        return false;
    }
};

inline const char* default_reexport_config_path() {
    return ".ast_distance_config.json";
}

inline std::string json_escape(const std::string& text) {
    std::string out;
    out.reserve(text.size());
    for (char c : text) {
        switch (c) {
            case '\\': out += "\\\\"; break;
            case '"': out += "\\\""; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    static constexpr char HEX[] = "0123456789abcdef";
                    out += "\\u00";
                    out += HEX[(static_cast<unsigned char>(c) >> 4) & 15];
                    out += HEX[static_cast<unsigned char>(c) & 15];
                } else out += c;
                break;
        }
    }
    return out;
}

inline std::string extract_json_string(const std::string& content, const std::string& key) {
    const std::string pattern = "\"" + key + "\"";
    size_t pos = content.find(pattern);
    if (pos == std::string::npos) return "";
    pos = content.find(':', pos + pattern.size());
    if (pos == std::string::npos) return "";
    pos = content.find('"', pos + 1);
    if (pos == std::string::npos) return "";

    std::string value;
    for (++pos; pos < content.size(); ++pos) {
        char c = content[pos];
        if (c == '"') break;
        if (c == '\\' && pos + 1 < content.size()) {
            char next = content[++pos];
            switch (next) {
                case 'n': value += '\n'; break;
                case 'r': value += '\r'; break;
                case 't': value += '\t'; break;
                default: value += next; break;
            }
        } else {
            value += c;
        }
    }
    return value;
}

inline std::string extract_json_object_body(const std::string& content, const std::string& key) {
    const std::string pattern = "\"" + key + "\"";
    size_t pos = content.find(pattern);
    if (pos == std::string::npos) return "";
    pos = content.find(':', pos + pattern.size());
    if (pos == std::string::npos) return "";
    pos = content.find('{', pos + 1);
    if (pos == std::string::npos) return "";

    int depth = 0;
    bool in_string = false;
    bool escaped = false;
    size_t start = pos + 1;
    for (size_t i = pos; i < content.size(); ++i) {
        char c = content[i];
        if (in_string) {
            if (escaped) {
                escaped = false;
            } else if (c == '\\') {
                escaped = true;
            } else if (c == '"') {
                in_string = false;
            }
            continue;
        }
        if (c == '"') {
            in_string = true;
        } else if (c == '{') {
            ++depth;
        } else if (c == '}') {
            --depth;
            if (depth == 0) {
                return content.substr(start, i - start);
            }
        }
    }
    return "";
}

inline bool load_reexport_config(const std::string& path, ReexportConfig& cfg) {
    cfg.config_path = path;
    cfg.type.clear();
    cfg.name.clear();
    cfg.source = {};
    cfg.target = {};
    cfg.loaded = false;
    cfg.had_parse_error = false;
    cfg.patterns.clear();

    std::ifstream file(path);
    if (!file.is_open()) return false;

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    cfg.type = extract_json_string(content, "type");
    cfg.name = extract_json_string(content, "name");
    std::string source_body = extract_json_object_body(content, "source");
    std::string target_body = extract_json_object_body(content, "target");
    cfg.source.path = extract_json_string(source_body, "path");
    cfg.source.lang = extract_json_string(source_body, "lang");
    cfg.target.path = extract_json_string(target_body, "path");
    cfg.target.lang = extract_json_string(target_body, "lang");

    const std::string key = "\"reexport_modules\"";
    size_t pos = content.find(key);
    if (pos == std::string::npos) {
        cfg.loaded = true;
        return true;
    }

    pos = content.find(':', pos + key.size());
    if (pos == std::string::npos) {
        cfg.had_parse_error = true;
        std::cerr << "Warning: " << path << " has reexport_modules but no ':' follows.\n";
        return true;
    }
    pos = content.find('[', pos);
    if (pos == std::string::npos) {
        cfg.had_parse_error = true;
        std::cerr << "Warning: " << path << " has reexport_modules but no '[' follows.\n";
        return true;
    }
    size_t end = content.find(']', pos);
    if (end == std::string::npos) {
        cfg.had_parse_error = true;
        std::cerr << "Warning: " << path << " has reexport_modules array with no closing ']'.\n";
        return true;
    }

    std::string array_body = content.substr(pos + 1, end - pos - 1);
    size_t i = 0;
    while (i < array_body.size()) {
        if (array_body[i] != '"') {
            ++i;
            continue;
        }
        ++i;
        std::string value;
        while (i < array_body.size() && array_body[i] != '"') {
            if (array_body[i] == '\\' && i + 1 < array_body.size()) {
                char next = array_body[i + 1];
                if (next == '"' || next == '\\' || next == '/') {
                    value += next;
                    i += 2;
                    continue;
                }
            }
            value += array_body[i++];
        }
        if (i < array_body.size()) ++i;
        if (!value.empty()) cfg.patterns.push_back(value);
    }

    cfg.loaded = true;
    return true;
}

inline bool write_ast_distance_config_stub(const std::string& path,
                                           const std::string& name,
                                           const ConfigEndpoint& source,
                                           const ConfigEndpoint& target) {
    if (std::filesystem::exists(path)) return false;

    std::ofstream out(path);
    if (!out.is_open()) return false;

    out << "{\n";
    out << "  \"type\": \"port\",\n";
    out << "  \"name\": \"" << json_escape(name.empty() ? "ast-distance-port" : name) << "\",\n";
    out << "  \"source\": {\n";
    out << "    \"path\": \"" << json_escape(source.path) << "\",\n";
    out << "    \"lang\": \"" << json_escape(source.lang) << "\"\n";
    out << "  },\n";
    out << "  \"target\": {\n";
    out << "    \"path\": \"" << json_escape(target.path) << "\",\n";
    out << "    \"lang\": \"" << json_escape(target.lang) << "\"\n";
    out << "  },\n";
    out << "  \"checks\": {\n";
    out << "    \"deep\": true,\n";
    out << "    \"missing\": true,\n";
    out << "    \"todos\": true,\n";
    out << "    \"lint\": true\n";
    out << "  },\n";
    out << "  \"reexport_modules\": [\n";
    out << "    \"mod.rs\",\n";
    out << "    \"lib.rs\"\n";
    out << "  ]\n";
    out << "}\n";
    return true;
}

} // namespace ast_distance
