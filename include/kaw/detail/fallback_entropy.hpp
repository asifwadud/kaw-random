#ifndef KAW_DETAIL_FALLBACK_ENTROPY_HPP
#define KAW_DETAIL_FALLBACK_ENTROPY_HPP

#include <array>
#include <chrono>
#include <cstdint>
#include <functional>
#include <limits>
#include <random>
#include <thread>

namespace kaw::random::detail {

inline auto fallback_entropy_values() {
  using value_type = std::random_device::result_type;

  auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
  auto thread_id = std::this_thread::get_id();
  volatile int local_var = 0;
  auto address = reinterpret_cast<std::uintptr_t>(&local_var);  // NOLINT

  std::random_device::result_type high = 0;
  if constexpr (sizeof(std::uintptr_t) > sizeof(value_type)) {
    constexpr auto shift_amount = sizeof(value_type) * std::numeric_limits<unsigned char>::digits;
    high = static_cast<value_type>(address >> shift_amount);
  }

  return std::array<value_type, 4>{static_cast<value_type>(now),
                                   static_cast<value_type>(std::hash<std::thread::id>{}(thread_id)),
                                   static_cast<value_type>(address), high};
}

}  // namespace kaw::random::detail

#endif  // KAW_DETAIL_FALLBACK_ENTROPY_HPP
