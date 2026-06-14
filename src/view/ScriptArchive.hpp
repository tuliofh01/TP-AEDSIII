#pragma once

// Binary archive of Lua view scripts.
// Stores multiple named Lua source entries in a single file
// and registers them all into a Lua state at runtime.

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

struct lua_State;

namespace project_view {

// A single named Lua script with an active/inactive flag.
struct ScriptEntry {
    bool active = true;
    std::string name;
    std::string source;
};

class ScriptArchive {
public:
    static constexpr uint32_t MAGIC = 0x4C554143;
    static constexpr uint32_t VERSION = 1;
    static constexpr uint32_t HEADER_SIZE = 32;

    // Persistence
    bool load(const std::string& path);
    bool save(const std::string& path) const;

    // Script management
    void add(const std::string& name, const std::string& source);
    bool remove(const std::string& name);
    bool contains(const std::string& name) const;
    size_t activeCount() const;

    // Lua registration
    void registerAll(lua_State* L) const;

    const std::vector<ScriptEntry>& entries() const { return entries_; }

private:
    std::vector<ScriptEntry> entries_;

    // Low-level I/O helpers
    static bool readExact(std::ifstream& file, char* buf, size_t n);
    static bool writeExact(std::ofstream& file, const char* buf, size_t n);
};

} // namespace project_view
