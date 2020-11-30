#pragma once

// TODO: Make this into a static class
namespace il2cpp_utils {
    // A hash function used to hash a pair of any kind
    struct hash_pair {
        template<class T1, class T2>
        size_t operator()(const std::pair<T1, T2>& p) const {
            auto hash1 = std::hash<T1>{}(p.first);
            auto hash2 = std::hash<T2>{}(p.second);
            return hash1 ^ hash2;
        }
    };
    // A hash function used to hash a pair of an object, pair
    struct hash_pair_3 {
        template<class T1, class T2, class T3>
        size_t operator()(const std::pair<T1, std::pair<T2, T3>>& p) const {
            auto hash1 = std::hash<T1>{}(p.first);
            auto hash2 = hash_pair{}(p.second);
            return hash1 ^ hash2;
        }
    };
}
