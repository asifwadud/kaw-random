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
    template <typename Engine = std::mt19937, typename T>
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


## 2. API & Distribution Consistency
- [ ] **Cross-Platform Deterministic Distributions**
  - Implement custom wrapper distributions (e.g., uniform integer/real) to replace standard library distributions (`std::uniform_int_distribution`) which have implementation-defined mapping algorithms and produce different outputs on GCC (libstdc++), Clang (libc++), and MSVC.
  - **Benefits**:
    - *Game Synchronization*: Enables lockstep multiplayer games across different clients (Windows/macOS/consoles) to stay in sync.
    - *Testing & Verification*: Unit tests asserting specific random sequences run reliably on any CI system.
    - *Replication*: Scientific simulations can be reproduced exactly on different compilers.
  - **Implementation**:
    - Use standardized compiler-independent algorithms (e.g., Lemire's division-less range mapping or rejection sampling) directly within the library.
