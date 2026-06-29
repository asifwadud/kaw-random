# kaw-random

A modern, production-grade C++20 header-only random number utility library under the `kaw::random` namespace.

Designed to be lightweight, thread-safe, and extremely simple to use compared to the standard C++ `<random>` library.

## Features

- **C++20 Concepts:** Strongly typed template constraints using C++20 standard concepts for clean compiler error messages.
- **Thread Safety:** Uses a `thread_local` Mersenne Twister engine (`std::mt19937`) to avoid state sharing and lock contention in multi-threaded environments.
- **Header-Only:** Zero compilation setup needed. Just include the header file and start coding.
- **Range Helpers:** Automatic compile-time distribution matching (integers vs. floating-point types).

## API & Usage

Include the library header:
```cpp
#include <kaw/random.hpp>
```

### 1. Stateful Generators & Functors (Type Aliases)
Generate random values by instantiating a stateful generator functor using type aliases under the `kaw` namespace. These can be called directly or passed to C++ standard algorithms (like `std::generate`).

* **`random_int`** generates random integers for a closed/inclusive range `[low, high]` [^1].
* **`random_bool`** generates random booleans based on a probability parameter [^2].

```cpp
using namespace kaw;

// 1. Basic Generation
random_int rand_int(1, 100);
int first = rand_int();       // e.g. 42
int second = rand_int();      // e.g. 7

random_bool rand_bool(0.5);   // 50% probability of true
bool val = rand_bool();       // e.g. true

// 2. Stateful Functor with Standard Algorithms
random_float rand_float(-100.0f, 100.0f);
std::vector<float> values(10);
std::generate(values.begin(), values.end(), rand_float);
```

[^1]: **Inclusive `[low, high]` for integers:** Maps to `std::uniform_int_distribution`. Standardized as closed by the C++ committee to prevent type overflow when representing the maximum possible value.
[^2]: **Specialized for bools:** Maps to `std::bernoulli_distribution`. Standardized as single probability parameter.

### 2. Convenience Free Functions
Generate random numbers on-the-fly without instantiating a generator class, using inclusive ranges for integers and half-open/exclusive ranges `[low, high)` for reals [^3]:
```cpp
// Free function calls directly in kaw::random
int val1 = kaw::random::get(1, 10);            // 1 to 10 inclusive
double val2 = kaw::random::get(0.0, 1.0);      // 0.0 to 1.0 (exclusive)
```

[^3]: **Exclusive `[low, high)` for reals:** Maps to `std::uniform_real_distribution`. Standardized as half-open by the C++ committee to match real math standards and avoid out-of-bounds indexing when scaling.

### 3. Generating Booleans (Bernoulli Distribution)
```cpp
bool heads = kaw::random::get_bool();          // Default 50% probability
bool lucky = kaw::random::get_bool(0.1);       // 10% chance of returning true
```

### 4. Filling an Existing Container
```cpp
std::vector<int> scores(100);
kaw::random::fill(scores, 1, 100);             // Fills vector in-place
```

### 5. Generating a New Container
```cpp
// Explicitly specify the desired container type as a template parameter
auto prices = kaw::random::generate<std::vector<double>>(50, 10.0, 50.0);
```


---

## Testing

To configure and run the Catch2 unit test suite:

```bash
# From within the RandomNumbers subdirectory
cmake -B build -S .
cmake --build build
cd build && ctest --verbose
```
