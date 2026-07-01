# Contributing to kaw-random

Thank you for your interest in contributing! This document provides guidelines for setting up the development environment, running unit tests, formatting/linting code, and working with our automated PR reviewer.

---

## 1. Local Setup & Testing

The library uses **Catch2** for unit testing. To configure the build system and run the test suite:

```bash
# Configure the build directory
cmake -B build -S .

# Build the project (includes tests)
cmake --build build

# Run all tests using CTest
cd build && ctest --verbose
```

---

## 2. Linting & Formatting

We enforce strict formatting and static analysis checks using `clang-format` and `clang-tidy` based on the C++ Core Guidelines.

### Prerequisites
Configure CMake to export compile commands, which is required by `clang-tidy` to resolve header search paths correctly:
```bash
cmake -B build -S . -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

### Formatting Guidelines
Before submitting a PR, make sure your code matches the project code style.
* **Format all source files in place**:
  ```bash
  cmake --build build --target format
  ```
* **Verify formatting without modifying files (Dry-Run)**:
  ```bash
  cmake --build build --target format-check
  ```

### Static Analysis
Run static analysis via `clang-tidy` to check for C++ best practices and potential bugs:
```bash
cmake --build build --target lint
```

---

## 3. Pull Request Guidelines & AI Reviews

This repository integrates with **Codium PR-Agent** to automate reviews on Pull Requests. 

### How It Works
* **Automatic Reviews**: When a PR is opened or updated, PR-Agent automatically analyzes the diff, formats a Conventional Commits-aligned description, and posts structural code suggestions.
* **Interactive Commands**: You can trigger specific tasks by commenting on your PR:
  * `/review` - Re-run the code review.
  * `/improve` - Request specific inline code improvements.
  * `/describe` - Regenerate the PR title and description.
  * `/ask <question>` - Ask a technical question about the diff.

### Custom Review Rules
Any contribution must comply with the custom rules configured for the PR-Agent:
1. **Thread-Safety**: Random number generation must use thread-local engines (`thread_local std::mt19937` or standard C++ architecture-appropriate engines) to prevent state sharing.
2. **C++20 Concepts**: Standard constraints should use C++20 concepts (e.g., `std::integral`, `std::floating_point`) instead of template metaprogramming/SFINAE tricks.
3. **Bound Consistency**: Integer range helpers must use closed (inclusive) bounds `[low, high]`, while real range helpers must use half-open (exclusive) bounds `[low, high)`.
4. **Header-Only Rules**: All non-template free functions declared in headers must be explicitly marked `inline`.
5. **Test Coverage**: Catch2 unit tests must be added or updated to cover any modifications.
