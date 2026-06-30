#ifndef KAW_DETAIL_SEEDING_STRATEGIES_HPP
#define KAW_DETAIL_SEEDING_STRATEGIES_HPP

#include <random>
#include <array>
#include <chrono>
#include <thread>
#include <cstdint>
#include <functional>

namespace kaw::random::detail {

enum class seeding_strategy {
    basic,
    balanced,
    full
};

// Select the active strategy at compile time
#if defined(KAW_RANDOM_SEED_BASIC) && defined(KAW_RANDOM_SEED_FULL)
#error "Cannot define both KAW_RANDOM_SEED_BASIC and KAW_RANDOM_SEED_FULL"
#endif

#if defined(KAW_RANDOM_SEED_BASIC)
constexpr seeding_strategy active_strategy = seeding_strategy::basic;
#elif defined(KAW_RANDOM_SEED_FULL)
constexpr seeding_strategy active_strategy = seeding_strategy::full;
#else
constexpr seeding_strategy active_strategy = seeding_strategy::balanced;
#endif

// Returns compile-time configured entropy element count
inline constexpr size_t get_seed_entropy_element_count() {
    if constexpr (active_strategy == seeding_strategy::basic) {
        return 1;
    } else if constexpr (active_strategy == seeding_strategy::full) {
        return std::mt19937::state_size; // 624
    } else { // balanced
        return 10;
    }
}

struct fallback_entropy {
    std::uint32_t now_entropy;
    std::uint32_t thread_entropy;
    std::uint32_t address_low;
    std::uint32_t address_high;
};

inline fallback_entropy get_fallback_entropy() {
    auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    auto thread_id = std::this_thread::get_id();
    volatile int local_var = 0;
    std::uintptr_t address = reinterpret_cast<std::uintptr_t>(&local_var);
    
    std::uint32_t high = 0;
    if constexpr (sizeof(std::uintptr_t) >= 8) {
        high = static_cast<std::uint32_t>(address >> 32);
    }
    
    return fallback_entropy {
        static_cast<std::uint32_t>(now),
        static_cast<std::uint32_t>(std::hash<std::thread::id>{}(thread_id)),
        static_cast<std::uint32_t>(address),
        high
    };
}

// Factory function to initialize the engine with the configured strategy
inline std::mt19937 create_seeded_engine() {
    if constexpr (active_strategy == seeding_strategy::basic) {
        std::random_device rd;
        std::uint32_t seed = rd();
        
        auto fallback = get_fallback_entropy();
        seed ^= fallback.now_entropy;
        seed ^= fallback.thread_entropy;
        seed ^= fallback.address_low;
        seed ^= fallback.address_high;
        
        return std::mt19937(seed);
    } 
    else if constexpr (active_strategy == seeding_strategy::full) {
        std::array<std::uint32_t, std::mt19937::state_size> seeds;
        std::random_device rd;
        for (auto& val : seeds) {
            val = rd();
        }
        
        auto fallback = get_fallback_entropy();
        seeds[0] ^= fallback.now_entropy;
        seeds[1] ^= fallback.thread_entropy;
        seeds[2] ^= fallback.address_low;
        seeds[3] ^= fallback.address_high;
        
        std::seed_seq seq(seeds.begin(), seeds.end());
        return std::mt19937(seq);
    } 
    else { // balanced (default)
        std::array<std::uint32_t, 10> seeds;
        std::random_device rd;
        for (size_t i = 0; i < 8; ++i) {
            seeds[i] = rd();
        }
        
        auto fallback = get_fallback_entropy();
        seeds[8] = fallback.now_entropy ^ fallback.address_low;
        seeds[9] = fallback.thread_entropy ^ fallback.address_high;
        
        std::seed_seq seq(seeds.begin(), seeds.end());
        return std::mt19937(seq);
    }
}

inline std::mt19937& get_thread_engine() {
    thread_local std::mt19937 engine = create_seeded_engine();
    return engine;
}

} // namespace kaw::random::detail

#endif // KAW_DETAIL_SEEDING_STRATEGIES_HPP
