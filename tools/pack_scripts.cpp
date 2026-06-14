#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "../src/view/ScriptArchive.hpp"

namespace fs = std::filesystem;

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: pack_scripts <input_dir> <output_file>\n";
        std::cerr << "  Recursively scans input_dir for .lua files and packs\n";
        std::cerr << "  them into a portable binary archive at output_file.\n";
        return 1;
    }

    std::string inputDir = argv[1];
    std::string outputFile = argv[2];

    if (!fs::is_directory(inputDir)) {
        std::cerr << "Error: " << inputDir << " is not a directory\n";
        return 1;
    }

    project_view::ScriptArchive archive;

    // Recursively collect all .lua files
    std::vector<fs::path> luaFiles;
    for (const auto& entry : fs::recursive_directory_iterator(inputDir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".lua") {
            luaFiles.push_back(entry.path());
        }
    }

    // Sort for deterministic output
    std::sort(luaFiles.begin(), luaFiles.end());

    for (const auto& path : luaFiles) {
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "  Warning: could not open " << path.filename() << "\n";
            continue;
        }
        std::stringstream ss;
        ss << file.rdbuf();
        std::string source = ss.str();

        // Relative path from input dir, with / replaced by .
        auto rel = fs::relative(path, inputDir).replace_extension("").string();
        for (auto& ch : rel) {
            if (ch == '/' || ch == '\\') ch = '.';
        }
        archive.add(rel, source);
        std::cout << "  Packed: " << rel << " (" << source.size() << " bytes)\n";
    }

    if (archive.activeCount() == 0) {
        std::cerr << "Error: no .lua files found in " << inputDir << "\n";
        return 1;
    }

    if (archive.save(outputFile)) {
        std::cout << "Wrote " << archive.activeCount() << " scripts to "
                  << outputFile << "\n";
        return 0;
    }

    std::cerr << "Error: failed to write " << outputFile << "\n";
    return 1;
}
