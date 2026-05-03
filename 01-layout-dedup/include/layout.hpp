#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <random>
#include <unordered_map>
#include "placement.hpp"

// Generates and caches a random 64-bit value for each unique placement
struct ZobristTable {
    std::mt19937_64 rng{1337}; // Fixed seed for reproducibility
    std::unordered_map<std::size_t, std::uint64_t> table;

    std::uint64_t get_value(const Placement& p) {
        std::size_t phash = PlacementHash{}(p);
        if (table.find(phash) == table.end()) {
            table[phash] = rng();
        }
        return table[phash];
    }

    static ZobristTable& get() {
        static ZobristTable instance;
        return instance;
    }
};

struct Layout {
    std::vector<Placement> placements;

    // Keep for exact equality check on collisions
    std::string canonical_string() const {
        std::vector<Placement> v = placements;
        std::sort(v.begin(), v.end(), [](const Placement& a, const Placement& b){
            if (a.loc.room != b.loc.room) return a.loc.room < b.loc.room;
            if (a.loc.wall != b.loc.wall) return a.loc.wall < b.loc.wall;
            if (a.artwork_id != b.artwork_id) return a.artwork_id < b.artwork_id;
            return a.orientation < b.orientation;
        });
        std::ostringstream out;
        for (const auto& p : v) {
            out << p.loc.room << "," << p.loc.wall << ","
                << p.artwork_id << "," << p.orientation << ";";
        }
        return out.str();
    }

    // TASK A: Zobrist Hashing (O(N) time, completely order-independent via XOR)
    std::size_t fast_zobrist_hash() const {
        std::uint64_t h = 0;
        for (const auto& p : placements) {
            h ^= ZobristTable::get().get_value(p);
        }
        return static_cast<std::size_t>(h);
    }

    // Equality operator handles any hash collisions seamlessly
    bool operator==(const Layout& other) const {
        return this->canonical_string() == other.canonical_string();
    }
};

struct LayoutHash {
    std::size_t operator()(const Layout& l) const {
        return l.fast_zobrist_hash();
    }
};