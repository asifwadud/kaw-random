#include <catch2/catch_test_macros.hpp>
#include <kaw/random.hpp>
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
      if (r_bool()) true_count++;
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
