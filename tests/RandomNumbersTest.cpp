#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <kaw/random.hpp>
#include <limits>
#include <random>
#include <set>
#include <thread>
#include <vector>

TEST_CASE("Random integer generation matches bounds", "[Random]") {
  int low = 1;
  int high = 10;
  for (int i = 0; i < 100; ++i) {
    int val = kaw::random::get(low, high);
    REQUIRE(val >= low);
    REQUIRE(val <= high);
  }
}

TEST_CASE("Random real generation matches bounds", "[Random]") {
  double low = 0.0;
  double high = 1.0;
  for (int i = 0; i < 100; ++i) {
    double val = kaw::random::get(low, high);
    REQUIRE(val >= low);
    REQUIRE(val < high);
  }
}

TEST_CASE("Random boolean generation", "[Random]") {
  int true_count = 0;
  for (int i = 0; i < 1000; ++i) {
    if (kaw::random::get_bool(0.5)) {
      true_count++;
    }
  }
  REQUIRE(true_count > 400);
  REQUIRE(true_count < 600);
}

TEST_CASE("Random container filling", "[Random]") {
  std::vector<int> vec(10);
  kaw::random::fill(vec, 1, 5);
  for (int val : vec) {
    REQUIRE(val >= 1);
    REQUIRE(val <= 5);
  }
}

TEST_CASE("Random container generation", "[Random]") {
  auto vec = kaw::random::generate<std::vector<double>>(10, 0.0, 10.0);
  REQUIRE(vec.size() == 10);
  for (double val : vec) {
    REQUIRE(val >= 0.0);
    REQUIRE(val < 10.0);
  }
}

TEST_CASE("Stateful gen functor usage", "[Random]") {
  kaw::random::gen<float> my_gen(1.0f, 5.0f);
  std::vector<float> vec(10);
  std::generate(vec.begin(), vec.end(), my_gen);
  for (float val : vec) {
    REQUIRE(val >= 1.0f);
    REQUIRE(val < 5.0f);
  }
}

TEST_CASE("Root-level type aliases and bool specialization", "[Random]") {
  SECTION("kaw::random_bool") {
    kaw::random_bool r_bool(0.5);
    int true_count = 0;
    for (int i = 0; i < 1000; ++i) {
      if (r_bool()) {
        true_count++;
      }
    }
    REQUIRE(true_count > 400);
    REQUIRE(true_count < 600);
  }

  SECTION("kaw::random_int32_t") {
    kaw::random_int32_t r_int32(5, 10);
    for (int i = 0; i < 100; ++i) {
      int val = r_int32();
      REQUIRE(val >= 5);
      REQUIRE(val <= 10);
    }
  }

  SECTION("kaw::random_double") {
    kaw::random_double r_double(0.0, 1.0);
    for (int i = 0; i < 100; ++i) {
      double val = r_double();
      REQUIRE(val >= 0.0);
      REQUIRE(val < 1.0);
    }
  }
}

TEST_CASE("Verify engine seed space matches the configured strategy", "[Random][Seeding]") {
  constexpr size_t seed_elements = kaw::random::detail::get_seed_entropy_element_count();
  constexpr size_t result_bits = std::numeric_limits<std::random_device::result_type>::digits;

#if defined(KAW_RANDOM_SEED_BASIC)
  SECTION("Basic Seeding Strategy") {
    REQUIRE(seed_elements == 1);
  }
#elif defined(KAW_RANDOM_SEED_FULL)
  SECTION("Full Seeding Strategy") {
    constexpr size_t expected_elements =
        (std::mt19937::state_size * std::mt19937::word_size + result_bits - 1) / result_bits;
    REQUIRE(seed_elements == expected_elements);
  }
#else  // default: balanced
  SECTION("Balanced Seeding Strategy") {
    constexpr size_t expected_elements = (256 + result_bits - 1) / result_bits;
    constexpr size_t min_elements = expected_elements < 4 ? 4 : expected_elements;
    REQUIRE(seed_elements == min_elements);
  }
#endif
}

TEST_CASE("High-quality seeding ensures multi-threaded uniqueness", "[Random][Seeding]") {
  const int num_threads = std::min<int>(kaw::random::detail::get_seed_entropy_element_count(), 16);

  std::vector<std::vector<int>> sequences(num_threads, std::vector<int>(5));

  {
    std::vector<std::jthread> threads;
    threads.reserve(num_threads);
    for (int i = 0; i < num_threads; ++i) {
      threads.emplace_back([&sequences, i]() {
        for (int j = 0; j < 5; ++j) {
          sequences[i][j] = kaw::random::get(0, 1000000);
        }
      });
    }
  }  // <-- All threads are guaranteed to be joined here!

  std::set<std::vector<int>> unique_sequences(sequences.begin(), sequences.end());

  // Verify all threads produced different random sequences, proving independent seeds
  REQUIRE(unique_sequences.size() == num_threads);
}

TEST_CASE("Normal/Gaussian distribution correctness", "[Random]") {
  SECTION("Stateful normal_gen functor") {
    kaw::random_normal_double norm(10.0, 2.0);
    constexpr int samples = 5000;
    double sum = 0.0;
    for (int i = 0; i < samples; ++i) {
      sum += norm();
    }
    double mean = sum / samples;
    // For mean = 10.0, stddev = 2.0, N = 5000, 99.9% confidence interval is well within 0.2
    REQUIRE(mean > 9.8);
    REQUIRE(mean < 10.2);
  }

  SECTION("get_normal and get_gaussian free functions") {
    constexpr int samples = 1000;
    double sum_normal = 0.0;
    double sum_gaussian = 0.0;
    for (int i = 0; i < samples; ++i) {
      sum_normal += kaw::random::get_normal(0.0, 1.0);
      sum_gaussian += kaw::random::get_gaussian(0.0, 1.0);
    }
    double mean_normal = sum_normal / samples;
    double mean_gaussian = sum_gaussian / samples;
    REQUIRE(mean_normal > -0.15);
    REQUIRE(mean_normal < 0.15);
    REQUIRE(mean_gaussian > -0.15);
    REQUIRE(mean_gaussian < 0.15);
  }

  SECTION("Container helpers for normal distributions") {
    auto vec = kaw::random::generate_normal<std::vector<double>>(100, 50.0, 5.0);
    REQUIRE(vec.size() == 100);
    double sum = 0.0;
    for (double val : vec) {
      sum += val;
    }
    double mean = sum / static_cast<double>(vec.size());
    REQUIRE(mean > 48.0);
    REQUIRE(mean < 52.0);

    std::vector<float> vec_float(100);
    kaw::random::fill_gaussian(vec_float, 0.0f, 1.0f);
    double sum_float = 0.0;
    for (float val : vec_float) {
      sum_float += val;
    }
    double mean_float = sum_float / static_cast<double>(vec_float.size());
    REQUIRE(mean_float > -0.5);
    REQUIRE(mean_float < 0.5);
  }
}
