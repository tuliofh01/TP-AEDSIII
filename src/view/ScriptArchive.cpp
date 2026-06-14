#include "ScriptArchive.hpp"

#include <fstream>
#include <iostream>
#include <cstring>
#include <lua.hpp>

namespace project_view {

// ========================================
// Binary I/O helpers
// ========================================
bool ScriptArchive::readExact(std::ifstream& file, char* buf, size_t n) {
    file.read(buf, static_cast<std::streamsize>(n));
    return file.gcount() == static_cast<std::streamsize>(n);
}

bool ScriptArchive::writeExact(std::ofstream& file, const char* buf, size_t n) {
    file.write(buf, static_cast<std::streamsize>(n));
    return file.good();
}

// ========================================
// Load archive from binary file
// ========================================
bool ScriptArchive::load(const std::string& path) {
    entries_.clear();

    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) return false;

    uint32_t magic, version, count;
    char reserved[20];

    if (!readExact(file, reinterpret_cast<char*>(&magic), 4)) return false;
    if (!readExact(file, reinterpret_cast<char*>(&version), 4)) return false;
    if (!readExact(file, reinterpret_cast<char*>(&count), 4)) return false;
    if (!readExact(file, reserved, 20)) return false;

    if (magic != MAGIC || version != VERSION) return false;

    for (uint32_t i = 0; i < count; ++i) {
        ScriptEntry entry;

        uint8_t status;
        uint16_t nameLen;
        uint32_t srcLen;

        if (!readExact(file, reinterpret_cast<char*>(&status), 1)) return false;
        if (!readExact(file, reinterpret_cast<char*>(&nameLen), 2)) return false;
        if (!readExact(file, reinterpret_cast<char*>(&srcLen), 4)) return false;

        entry.active = (status == 'A');

        entry.name.resize(nameLen);
        if (nameLen > 0) {
            if (!readExact(file, entry.name.data(), nameLen)) return false;
        }

        entry.source.resize(srcLen);
        if (srcLen > 0) {
            if (!readExact(file, entry.source.data(), srcLen)) return false;
        }

        entries_.push_back(std::move(entry));
    }

    return true;
}

// ========================================
// Save archive to binary file
// ========================================
bool ScriptArchive::save(const std::string& path) const {
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file.is_open()) return false;

    uint32_t count = 0;
    for (const auto& e : entries_) {
        if (e.active) ++count;
    }

    uint32_t magic = MAGIC;
    uint32_t version = VERSION;
    char reserved[20]{};

    if (!writeExact(file, reinterpret_cast<const char*>(&magic), 4)) return false;
    if (!writeExact(file, reinterpret_cast<const char*>(&version), 4)) return false;
    if (!writeExact(file, reinterpret_cast<const char*>(&count), 4)) return false;
    if (!writeExact(file, reserved, 20)) return false;

    for (const auto& e : entries_) {
        if (!e.active) continue;

        uint8_t status = 'A';
        uint16_t nameLen = static_cast<uint16_t>(e.name.size());
        uint32_t srcLen = static_cast<uint32_t>(e.source.size());

        if (!writeExact(file, reinterpret_cast<const char*>(&status), 1)) return false;
        if (!writeExact(file, reinterpret_cast<const char*>(&nameLen), 2)) return false;
        if (!writeExact(file, reinterpret_cast<const char*>(&srcLen), 4)) return false;
        if (nameLen > 0 && !writeExact(file, e.name.data(), nameLen)) return false;
        if (srcLen > 0 && !writeExact(file, e.source.data(), srcLen)) return false;
    }

    return true;
}

void ScriptArchive::add(const std::string& name, const std::string& source) {
    for (auto& e : entries_) {
        if (e.name == name) {
            e.active = true;
            e.source = source;
            return;
        }
    }
    entries_.emplace_back(true, name, source);
}

bool ScriptArchive::remove(const std::string& name) {
    for (auto& e : entries_) {
        if (e.name == name) {
            e.active = false;
            return true;
        }
    }
    return false;
}

bool ScriptArchive::contains(const std::string& name) const {
    for (const auto& e : entries_) {
        if (e.name == name && e.active) return true;
    }
    return false;
}

size_t ScriptArchive::activeCount() const {
    size_t n = 0;
    for (const auto& e : entries_) {
        if (e.active) ++n;
    }
    return n;
}

void ScriptArchive::registerAll(lua_State* L) const {
    // Populate package.preload so require() resolves dependencies on demand
    lua_getglobal(L, "package");

    lua_getfield(L, -1, "preload");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_setfield(L, -3, "preload");
    }

    for (const auto& entry : entries_) {
        if (!entry.active) continue;

        if (luaL_loadstring(L, entry.source.c_str()) != LUA_OK) {
            std::cerr << "Lua: erro ao carregar " << entry.name << ": "
                      << lua_tostring(L, -1) << std::endl;
            lua_pop(L, 1);
            continue;
        }
        lua_setfield(L, -2, entry.name.c_str());
    }

    lua_pop(L, 2);

    // Require router — triggers on-demand loading of all dependencies
    lua_getglobal(L, "require");
    lua_pushstring(L, "handlers.router");
    if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
        std::cerr << "Lua: erro ao carregar router: "
                  << lua_tostring(L, -1) << std::endl;
        lua_pop(L, 1);
    }

    // Verify RenderUI is now globally available
    lua_getglobal(L, "RenderUI");
    if (lua_isnil(L, -1)) {
        std::cerr << "Lua: RenderUI nao definida pelo router" << std::endl;
    }
    lua_pop(L, 1);
}

} // namespace project_view
