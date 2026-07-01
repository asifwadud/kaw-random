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

## Testing

To configure and run the Catch2 unit test suite:

```bash
# From within the RandomNumbers subdirectory
cmake -B build -S .
cmake --build build
cd build && ctest --verbose
```

## Linting & Formatting

The project enforces code formatting and static analysis checks using `clang-format` and `clang-tidy` based on the C++ Core Guidelines.

### Prerequisites

Configure CMake to export compile commands (required by `clang-tidy` to resolve header paths):
```bash
cmake -B build -S . -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

### Formatting

* **Format all source files in place**:
  ```bash
  cmake --build build --target format
  ```
* **Verify formatting without modifying files (Dry-Run)**:
  ```bash
  cmake --build build --target format-check
  ```

### Static Analysis

* **Run static analysis via clang-tidy**:
  ```bash
  cmake --build build --target lint
  ```

## AI Code Reviews

This repository integrates with **Codium PR-Agent** to provide automated, agentic code reviews on Pull Requests. The review process is optimized for our modern C++20 and thread-safety guidelines.

### How It Works
- **Automatic Triggers:** When a new Pull Request is opened or updated, PR-Agent automatically analyzes the diff, formats a clean PR description, and provides structural code suggestions/reviews.
- **Interactive Commands:** You can comment on your Pull Request using commands to trigger specific tasks. Some useful commands include:
  - `/review` - Ask the agent to re-run the code review.
  - `/improve` - Request specific inline code improvement suggestions.
  - `/describe` - Regenerate the Pull Request title and description based on the code changes.
  - `/ask <question>` - Ask questions about the Pull Request diff.

### Custom Review Guidelines
PR-Agent is configured to check for compliance with:
- Thread-safe random number generation (`thread_local std::mt19937` engines).
- C++20 concepts (`std::integral`, `std::floating_point` instead of SFINAE template tricks).
- Closed distribution bounds for integers, half-open distribution bounds for real numbers.
- Header-only rules (ensuring non-template free functions are `inline`).
- Catch2 test updates on code modifications.

