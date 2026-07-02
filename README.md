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
* **`random_normal_double` / `random_normal_float`** generates normally (Gaussian) distributed floating-point numbers based on mean and standard deviation parameters.

```cpp
using namespace kaw;

// 1. Basic Generation
random_int rand_int(1, 100);
int first = rand_int();       // e.g. 42
int second = rand_int();      // e.g. 7

random_bool rand_bool(0.5);   // 50% probability of true
bool val = rand_bool();       // e.g. true

random_normal_double rand_norm(0.0, 1.0); // Standard normal distribution (mean=0, stddev=1)
double val_norm = rand_norm();            // e.g. -0.42

// 2. Stateful Functor with Standard Algorithms
random_float rand_float(-100.0f, 100.0f);
std::vector<float> values(10);
std::generate(values.begin(), values.end(), rand_float);
```

[^1]: **Inclusive `[low, high]` for integers:** Maps to `std::uniform_int_distribution`. Standardized as closed by the C++ committee to prevent type overflow when representing the maximum possible value.
[^2]: **Specialized for bools:** Maps to `std::bernoulli_distribution`. Standardized as single probability parameter.

### 2. Convenience Free Functions
Generate random numbers on-the-fly without instantiating a generator class, using inclusive ranges for integers, half-open/exclusive ranges `[low, high)` for reals [^3], Bernoulli trials for booleans, and normal (Gaussian) distributions:
```cpp
// Generate integers (inclusive) or reals (exclusive)
int val1 = kaw::random::get(1, 10);            // 1 to 10 inclusive
double val2 = kaw::random::get(0.0, 1.0);      // 0.0 to 1.0 (exclusive)

// Generate booleans (Bernoulli distribution)
bool heads = kaw::random::get_bool();          // Default 50% probability
bool lucky = kaw::random::get_bool(0.1);       // 10% chance of returning true

// Generate normally distributed values
double val3 = kaw::random::get_normal(0.0, 1.0); // Mean = 0.0, stddev = 1.0
double val4 = kaw::random::get_gaussian(10.0, 2.0); // Alias for get_normal
```

[^3]: **Exclusive `[low, high)` for reals:** Maps to `std::uniform_real_distribution`. Standardized as half-open by the C++ committee to match real math standards and avoid out-of-bounds indexing when scaling.

### 3. Filling an Existing Container
```cpp
std::vector<int> scores(100);
kaw::random::fill(scores, 1, 100);             // Fills vector in-place

std::vector<double> noise(100);
kaw::random::fill_normal(noise, 0.0, 1.0);     // Fills with normal values [^4]
```

### 4. Generating a New Container
```cpp
// Explicitly specify the desired container type as a template parameter
auto prices = kaw::random::generate<std::vector<double>>(50, 10.0, 50.0);

// Generate a container with normally distributed values
auto offsets = kaw::random::generate_normal<std::vector<double>>(50, 0.0, 0.5);
```

[^4]: **Gaussian helper aliases:** You can also use `fill_gaussian` and `generate_gaussian` as direct aliases.


## Performance Guidelines

While the library provides convenience free functions for quick, on-the-fly number generation, they are **slightly less efficient** than instantiating stateful generator functors in high-performance loops:

1. **Thread-Local Lookup Overhead**:
   * Free functions (`get`, `get_bool`, `get_normal`) must perform a thread-local storage (TLS) lookup on every single call to locate the current thread's random engine and persistent distribution cache.
   * Stateful functors (like `random_int` or `random_normal_double`) resolve the engine lookup only once (or locally) and hold the distribution state directly as class members, avoiding TLS overhead in loops.
2. **Parameter Re-Evaluation (`param_type`)**:
   * Free functions must copy and update the distribution's parameter configuration (using `std::normal_distribution::param_type`) on every call to support changing arguments dynamically.
   * Stateful functors construct the distribution parameters exactly once.
3. **Box-Muller Caching for Normal Distributions**:
   * Normally distributed values are generated in pairs (Box-Muller transform). The second value is cached internally inside the distribution object.
   * Stateful functors (`random_normal_double`) preserve this cache across successive calls. While free functions also cache them thread-locally, changing the parameters (`mean` or `stddev`) dynamically between calls will invalidate and discard this cache.

> [!TIP]
> Use **convenience free functions** for simple, one-off random queries (e.g. game setup, picking a server path, generating a unique ID on start).
> Use **stateful functors** (`kaw::random_int`, `kaw::random_normal_double`) inside tight loops or when passing to standard library algorithms (e.g. `std::generate`).


## Seeding & Strategies

To ensure high-quality random numbers and prevent seed collisions across concurrent threads and execution environments, `kaw-random` uses a consolidated `thread_local std::mt19937` engine per thread.

The library supports three seeding strategies, selectable at compile time using preprocessor macros:

| Strategy | Macro | Seed Size | Description | Initialization Performance |
| :--- | :--- | :--- | :--- | :--- |
| **Balanced** *(Default)* | *(none)* | 8 elements of `std::random_device::result_type` (typically 256 bits) | 8 elements of true entropy mixed with fallback time, thread ID, and stack address offsets. Resilient against deterministic/broken platform `std::random_device` implementations. | Very Fast (8 `std::random_device` queries) |
| **Basic** | `KAW_RANDOM_SEED_BASIC` | 1 element of `std::random_device::result_type` (typically 32 bits) | Legacy single-seed method. | Ultra Fast (1 `std::random_device` query) |
| **Full** | `KAW_RANDOM_SEED_FULL` | 624 elements of `std::random_device::result_type` (typically 19,968 bits) | Fills the entire internal state of the Mersenne Twister engine. | Slow (624 `std::random_device` queries) |

### How to Configure

Define the macro in your project before including `kaw/random.hpp`:

```cpp
#define KAW_RANDOM_SEED_FULL
#include <kaw/random.hpp>
```

Or configure it globally in your build system (e.g. CMake):

```cmake
target_compile_definitions(your_target PRIVATE KAW_RANDOM_SEED_FULL)
```

---

## Contributing

Contributions are welcome! Please see the [Contributing Guidelines](CONTRIBUTING.md) for details on setting up the local build system, running unit tests, formatting/linting code, and working with our pull request review workflow.

