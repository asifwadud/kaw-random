#ifndef KAW_DETAIL_SEEDING_STRATEGIES_HPP
#define KAW_DETAIL_SEEDING_STRATEGIES_HPP

#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <kaw/detail/fallback_entropy.hpp>
#include <limits>
#include <random>
#include <tuple>

namespace kaw::random::detail {

enum class seeding_strategy : std::uint8_t { basic, balanced, full };

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
constexpr size_t get_seed_entropy_element_count() {
  constexpr size_t result_bits = std::numeric_limits<std::random_device::result_type>::digits;
  if constexpr (active_strategy == seeding_strategy::basic) {
    return 1;  // NOLINT
  } else if constexpr (active_strategy == seeding_strategy::full) {
    // std::mt19937 requires state_size elements of word_size width
    constexpr size_t target_bits = std::mt19937::state_size * std::mt19937::word_size;
    return (target_bits + result_bits - 1) / result_bits;
  } else {  // balanced
    // balanced targets 256 bits of entropy
    constexpr size_t target_bits = 256;
    constexpr size_t required = (target_bits + result_bits - 1) / result_bits;
    // Must be at least the size of fallback entropy fields (4)
    return required < 4 ? 4 : required;
  }
}

template <std::size_t size>
inline auto make_random_device_seeds() {
  std::array<std::random_device::result_type, size> device_seeds{};
  std::random_device rand_dev;
  std::ranges::generate(device_seeds, std::ref(rand_dev));
  return device_seeds;
}

template <std::size_t size>
inline void mix_fallback_entropy(std::array<std::random_device::result_type, size>& seeds) {
  auto fallback_values = fallback_entropy_values();
  static_assert(size >= std::tuple_size_v<decltype(fallback_entropy_values())>,
                "seed array must have at least one slot per fallback_entropy field");

  std::ranges::transform(fallback_values, seeds, seeds.begin(), std::bit_xor<>());
}

// Factory function to initialize the engine with the configured strategy
inline std::mt19937 create_seeded_engine() {
  if constexpr (active_strategy == seeding_strategy::basic) {
    auto seed = std::random_device{}();

    std::ranges::for_each(fallback_entropy_values(), [&seed](auto value) { seed ^= value; });

    return std::mt19937(seed);
  } else {  // for both balanced (default) and full seeding strategy, differs only in seed_seq size
    auto seeds = make_random_device_seeds<get_seed_entropy_element_count()>();
    mix_fallback_entropy(seeds);

    std::seed_seq seq(seeds.begin(), seeds.end());
    return std::mt19937(seq);
  }
}

inline std::mt19937& get_thread_engine() {
  thread_local std::mt19937 engine = create_seeded_engine();
  return engine;
}

}  // namespace kaw::random::detail

#endif  // KAW_DETAIL_SEEDING_STRATEGIES_HPP
