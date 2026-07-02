#ifndef KAW_RANDOM_HPP
#define KAW_RANDOM_HPP

#include <algorithm>
#include <concepts>
#include <cstdint>
#include <kaw/detail/seeding_strategies.hpp>
#include <random>
#include <type_traits>
#include <vector>

#if __has_include(<stdfloat>)
#include <stdfloat>
#endif

namespace kaw::random {

// TODO: Consider selecting std::mt19937 or std::mt19937_64 dynamically based on CPU word size
// (e.g., using sizeof(void*) == 8 or INTPTR_MAX == INT64_MAX). Note that doing so will break
// cross-architecture reproducibility (32-bit vs 64-bit outputs will differ), but it optimizes
// performance on 64-bit systems.

// Compile-time distribution selection constraint
template <typename T>
using random_dist_t = std::conditional_t<std::is_integral_v<T>, std::uniform_int_distribution<T>,
                                         std::uniform_real_distribution<T>>;

// Stateful Functor Class (useful for std::generate, etc.)
template <typename T>
class gen {
public:
  using dist_type = random_dist_t<T>;

  gen(T low, T high) : distribution(low, high) {}

  T operator()() { return distribution(get_engine()); }

private:
  static std::mt19937& get_engine() { return detail::get_thread_engine(); }
  dist_type distribution;
};

// Stateful Functor Specialization for bool (using Bernoulli Distribution)
template <>
class gen<bool> {
public:
  gen(double probability = 0.5) : distribution(probability) {}

  bool operator()() { return distribution(get_engine()); }

private:
  static std::mt19937& get_engine() { return detail::get_thread_engine(); }
  std::bernoulli_distribution distribution;
};

// Stateful Functor for Normal (Gaussian) Distribution
template <typename T = double>
  requires std::floating_point<T>
class normal_gen {
public:
  using dist_type = std::normal_distribution<T>;

  normal_gen(T mean = 0.0, T stddev = 1.0) : distribution(mean, stddev) {}

  T operator()() { return distribution(get_engine()); }

private:
  static std::mt19937& get_engine() { return detail::get_thread_engine(); }
  dist_type distribution;
};

// --- Free Functions ---

// Generate a random number between low and high (inclusive for integers, half-open for reals)
template <typename T>
  requires std::integral<T> || std::floating_point<T>
T get(T low, T high) {
  using dist_t = random_dist_t<T>;
  thread_local static dist_t dist;
  typename dist_t::param_type params(low, high);
  return dist(detail::get_thread_engine(), params);
}

// Generate a random boolean (default 50% probability)
inline bool get_bool(double probability = 0.5) {
  thread_local static std::bernoulli_distribution dist;
  std::bernoulli_distribution::param_type params(probability);
  return dist(detail::get_thread_engine(), params);
}

// Generate a normally (Gaussian) distributed value
template <typename T = double>
  requires std::floating_point<T>
inline T get_normal(T mean = 0.0, T stddev = 1.0) {
  thread_local static std::normal_distribution<T> dist;
  thread_local static T last_mean = 0.0;
  thread_local static T last_stddev = 0.0;
  thread_local static bool has_last = false;

  if (!has_last || last_mean != mean || last_stddev != stddev) {
    dist.param(typename std::normal_distribution<T>::param_type(mean, stddev));
    dist.reset();
    last_mean = mean;
    last_stddev = stddev;
    has_last = true;
  }

  return dist(detail::get_thread_engine());
}

template <typename T = double>
  requires std::floating_point<T>
inline T get_gaussian(T mean = 0.0, T stddev = 1.0) {
  return get_normal<T>(mean, stddev);
}

// Fill an existing container in-place
// TODO: Consider using C++20 std::ranges::generate(container, generator) here instead of a manual
// loop.
template <typename Container>
void fill(Container& container, typename Container::value_type low,
          typename Container::value_type high) {
  using T = typename Container::value_type;
  gen<T> generator(low, high);
  for (auto& element : container) {
    element = generator();
  }
}

// Generate a new container of a given size
template <typename Container>
Container generate(size_t size, typename Container::value_type low,
                   typename Container::value_type high) {
  Container container(size);
  fill(container, low, high);
  return container;
}

// Fill an existing container with normally distributed values in-place
template <typename Container>
  requires std::floating_point<typename Container::value_type>
void fill_normal(Container& container, typename Container::value_type mean = 0.0,
                 typename Container::value_type stddev = 1.0) {
  using T = typename Container::value_type;
  normal_gen<T> generator(mean, stddev);
  for (auto& element : container) {
    element = generator();
  }
}

template <typename Container>
  requires std::floating_point<typename Container::value_type>
void fill_gaussian(Container& container, typename Container::value_type mean = 0.0,
                   typename Container::value_type stddev = 1.0) {
  fill_normal(container, mean, stddev);
}

// Generate a new container filled with normally distributed values
template <typename Container>
  requires std::floating_point<typename Container::value_type>
Container generate_normal(size_t size, typename Container::value_type mean = 0.0,
                          typename Container::value_type stddev = 1.0) {
  Container container(size);
  fill_normal(container, mean, stddev);
  return container;
}

template <typename Container>
  requires std::floating_point<typename Container::value_type>
Container generate_gaussian(size_t size, typename Container::value_type mean = 0.0,
                            typename Container::value_type stddev = 1.0) {
  return generate_normal<Container>(size, mean, stddev);
}

// TODO: Add a helper function for shuffling ranges/containers using std::ranges::shuffle under the
// hood: template <typename Range> void shuffle(Range& r) {
//   std::ranges::shuffle(r, get_engine());
// }

}  // namespace kaw::random

namespace kaw {
// basic types
using random_bool = random::gen<bool>;
using random_char = random::gen<char>;
using random_schar = random::gen<signed char>;
using random_uchar = random::gen<unsigned char>;
using random_short = random::gen<short>;
using random_ushort = random::gen<unsigned short>;
using random_int = random::gen<int>;
using random_uint = random::gen<unsigned int>;
using random_long = random::gen<long>;
using random_ulong = random::gen<unsigned long>;
using random_long_long = random::gen<long long>;
using random_ulong_long = random::gen<unsigned long long>;
using random_float = random::gen<float>;
using random_double = random::gen<double>;
using random_long_double = random::gen<long double>;

// Normal / Gaussian distribution aliases
using random_normal_float = random::normal_gen<float>;
using random_normal_double = random::normal_gen<double>;
using random_normal_long_double = random::normal_gen<long double>;

// cstdint types
using random_int8_t = random::gen<std::int8_t>;
using random_uint8_t = random::gen<std::uint8_t>;
using random_int16_t = random::gen<std::int16_t>;
using random_uint16_t = random::gen<std::uint16_t>;
using random_int32_t = random::gen<std::int32_t>;
using random_uint32_t = random::gen<std::uint32_t>;
using random_int64_t = random::gen<std::int64_t>;
using random_uint64_t = random::gen<std::uint64_t>;
using random_size_t = random::gen<std::size_t>;

// stdfloat types (conditionally compiled using standard macros)
#ifdef __STDCPP_FLOAT16_T__
using random_float16_t = random::gen<std::float16_t>;
using random_normal_float16_t = random::normal_gen<std::float16_t>;
#endif
#ifdef __STDCPP_FLOAT32_T__
using random_float32_t = random::gen<std::float32_t>;
using random_normal_float32_t = random::normal_gen<std::float32_t>;
#endif
#ifdef __STDCPP_FLOAT64_T__
using random_float64_t = random::gen<std::float64_t>;
using random_normal_float64_t = random::normal_gen<std::float64_t>;
#endif
#ifdef __STDCPP_FLOAT128_T__
using random_float128_t = random::gen<std::float128_t>;
using random_normal_float128_t = random::normal_gen<std::float128_t>;
#endif
#ifdef __STDCPP_BFLOAT16_T__
using random_bfloat16_t = random::gen<std::bfloat16_t>;
using random_normal_bfloat16_t = random::normal_gen<std::bfloat16_t>;
#endif
}  // namespace kaw

#endif  // KAW_RANDOM_HPP
