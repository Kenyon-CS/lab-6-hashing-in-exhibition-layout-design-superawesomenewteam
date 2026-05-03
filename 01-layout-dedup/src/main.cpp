#include <iostream>
#include <fstream>
#include <unordered_set>
#include <vector>
#include <string>
#include <chrono>

#include "layout.hpp"

static bool parse_layouts(const std::string& path, std::vector<Layout>& out) {
    std::ifstream in(path);
    if (!in) return false;

    Layout cur;
    std::string line;
    while (std::getline(in, line)) {
        if (line.size() == 0) {
            if (!cur.placements.empty()) {
                out.push_back(cur);
                cur.placements.clear();
            }
            continue;
        }
        Placement p;
        if (!(std::istringstream(line) >> p.loc.room >> p.loc.wall >> p.artwork_id >> p.orientation)) {
            std::cerr << "Bad line: " << line << "\n";
            return false;
        }
        cur.placements.push_back(p);
    }
    if (!cur.placements.empty()) out.push_back(cur);
    return true;
}

int main(int argc, char** argv) {
    std::string file = (argc >= 2) ? argv[1] : "data/large_layouts.txt";

    std::vector<Layout> layouts;
    if (!parse_layouts(file, layouts)) {
        std::cerr << "Failed to read " << file << "\n";
        return 1;
    }

    std::cout << "Layouts read: " << layouts.size() << "\n\n";

    // --- OLD SLOW METHOD (String Hashing) ---
    auto start_slow = std::chrono::high_resolution_clock::now();
    std::unordered_set<std::string> seen_slow;
    int duplicates_slow = 0;
    for (std::size_t i = 0; i < layouts.size(); i++) {
        const std::string key = layouts[i].canonical_string();
        if (seen_slow.find(key) != seen_slow.end()) {
            duplicates_slow++;
        } else {
            seen_slow.insert(key);
        }
    }
    auto end_slow = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> ms_slow = end_slow - start_slow;

    // --- NEW FAST METHOD (Zobrist Hashing) ---
    auto start_fast = std::chrono::high_resolution_clock::now();
    std::unordered_set<Layout, LayoutHash> seen_fast;
    int duplicates_fast = 0;
    for (std::size_t i = 0; i < layouts.size(); i++) {
        if (seen_fast.find(layouts[i]) != seen_fast.end()) {
            duplicates_fast++;
        } else {
            seen_fast.insert(layouts[i]);
        }
    }
    auto end_fast = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> ms_fast = end_fast - start_fast;

    // --- RESULTS ---
    std::cout << "=== PERFORMANCE RESULTS ===\n";
    std::cout << "Unique layouts: " << seen_fast.size() << "\n";
    std::cout << "Duplicates:     " << duplicates_fast << "\n\n";

    std::cout << "Slow Method (String Hashing) Time:  " << ms_slow.count() << " ms\n";
    std::cout << "Fast Method (Zobrist Hashing) Time: " << ms_fast.count() << " ms\n";
    std::cout << "Speedup: " << ms_slow.count() / ms_fast.count() << "x faster!\n";

    return 0;
}