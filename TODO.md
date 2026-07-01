# kaw-random TODO list

A collection of planned features, refactoring goals, and design improvements for the library.

## 1. Engine & Seeding Enhancements
- [ ] **Host-Optimized Default Engine**
  - Select `std::mt19937_64` as the default engine on 64-bit systems (e.g., using `sizeof(void*) == 8` checks) and fallback to `std::mt19937` on 32-bit systems.
  - Ensures 64-bit random values are generated in a single engine step without sacrificing performance on 32-bit platforms.
  - Keeps a single `thread_local` engine per thread to maintain sequence predictability.
- [ ] **Alternative Engine Support**
  - Add optional support/wrappers for lightweight, high-performance engines like PCG (`pcg32`/`pcg64`), Xoshiro, or `WyRand`.
  - Decouple the library API from `std::mt19937` by templating the generator wrapper:
    ```cpp
    template <typename T, typename Engine = std::mt19937>
    T get(T low, T high);
    ```
  - Support compile-time configuration of the default engine (e.g., `#define KAW_RANDOM_DEFAULT_ENGINE_PCG` / `-DKAW_RANDOM_DEFAULT_ENGINE_PCG` in CMake).
  - Maintain compatibility with standard C++ `UniformRandomBitGenerator` concepts.
  - **Benefits**:
    - *Reduced Memory Footprint*: PCG/Xoshiro require a tiny state (16–64 bytes) compared to `std::mt19937`'s 2.5 KB state, enabling thousands of active streams in memory-constrained environments.
    - *Speed & Statistical Quality*: Modern engines run significantly faster and pass strict TestU01 BigCrush checks (which `std::mt19937` fails).
- [ ] **Simplify Seeding (Keep Balanced Strategy Only)**
  - Remove the `Basic` (32-bit single seed) and `Full` (624-word state seeding) compile-time seeding strategies.
  - Standardize exclusively on the `Balanced` strategy (256 bits of true entropy + fallback system entropy).
  - **Benefits**:
    - *Code & Build Simplification*: Deletes preprocessor macros, configuration enums, static branch conditionals, and complex multiple test binaries (`kaw_random_tests_basic`/`kaw_random_tests_full`) in `CMakeLists.txt` and `cmake.yml`.
    - *Enforced Quality*: Eliminates the risk of users accidentally configuring under-seeded engines (Basic) or incurring unnecessary startup latency overhead (Full).
- [ ] **Abstractions for Seeding (Entropy Sources & Collector)**
  - Refactor `fallback_entropy.hpp` and the seeding pipeline to use a modular C++20 `entropy_source` concept.
  - Implement individual, highly testable sources (e.g. `random_device_source`, `system_time_source`, `thread_id_source`, `stack_address_source`).
  - Introduce a compile-time `entropy_collector` that aggregates and mixes entropy from configured sources.
  - **Compile-time Fold Mixing**: Handle differing bit-widths of various entropy sources (e.g. 64-bit stack pointers or time ticks vs. 32-bit `random_device` results) safely using compile-time chunk folding (`fold_entropy`).
  - **Benefits**:
    - *Extensibility*: Easily plug in new/custom entropy sources on specific platforms (e.g. process ID, network card stats, or hardware entropy).
    - *Code Quality*: Isolates unsafe pointer cast warnings (`reinterpret_cast` for stack pointer) to a single struct (`stack_address_source`) with local warning suppressions (`// NOLINT`).
    - *No Loss of Entropy*: Ensures all bits of larger sources contribute fully to smaller target types without truncation or padding issues.



## 2. API & Distribution Consistency
- [ ] **Cross-Platform Deterministic Distributions**
  - Implement custom wrapper distributions (e.g., uniform integer/real) to replace standard library distributions (`std::uniform_int_distribution`) which have implementation-defined mapping algorithms and produce different outputs on GCC (libstdc++), Clang (libc++), and MSVC.
  - **Benefits**:
    - *Game Synchronization*: Enables lockstep multiplayer games across different clients (Windows/macOS/consoles) to stay in sync.
    - *Testing & Verification*: Unit tests asserting specific random sequences run reliably on any CI system.
    - *Replication*: Scientific simulations can be reproduced exactly on different compilers.
  - **Implementation**:
    - Use standardized compiler-independent algorithms (e.g., Lemire's division-less range mapping or rejection sampling) directly within the library.
