cmake_minimum_required(VERSION 3.14)
project(kaw_random VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define header-only library target
add_library(kaw_random INTERFACE)
target_include_directories(kaw_random INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

enable_testing()

# Enable testing option
option(BUILD_TESTING "Build unit tests" ON)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

# Formatting and Linting Integration

# 1. Search for clang-format (prioritizing version 20 to match development/CI tooling)
find_program(CLANG_FORMAT_EXE NAMES clang-format-20 clang-format)

# Recursively glob quality files. CONFIGURE_DEPENDS triggers re-globbing if files are added/removed.
file(GLOB_RECURSE QUALITY_FILES CONFIGURE_DEPENDS
    "include/*.hpp"
    "include/*.h"
    "tests/*.cpp"
    "tests/*.hpp"
    "tests/*.h"
)

# Define format and format-check targets. If clang-format is missing, define fallback error targets.
if(CLANG_FORMAT_EXE)
    # Target to format files in place
    add_custom_target(format
        COMMAND ${CLANG_FORMAT_EXE} -i ${QUALITY_FILES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Applying clang-format style corrections..."
    )
    # Target to perform formatting validation (dry-run) without editing files
    add_custom_target(format-check
        COMMAND ${CLANG_FORMAT_EXE} --dry-run --Werror ${QUALITY_FILES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Validating C++ code formatting..."
    )
else()
    # Fallback to display error and fail the build if clang-format is missing
    add_custom_target(format
        COMMAND ${CMAKE_COMMAND} -E echo "Error: clang-format was not found in PATH."
        COMMAND ${CMAKE_COMMAND} -E false
        COMMENT "Failed: clang-format missing"
    )
    add_custom_target(format-check
        COMMAND ${CMAKE_COMMAND} -E echo "Error: clang-format was not found in PATH."
        COMMAND ${CMAKE_COMMAND} -E false
        COMMENT "Failed: clang-format missing"
    )
endif()

# 2. Search for clang-tidy (prioritizing version 20 to match development/CI tooling)
find_program(CLANG_TIDY_EXE NAMES clang-tidy-20 clang-tidy)

# Define lint target. Requires clang-tidy to be found and BUILD_TESTING to be enabled (for compilation database TUs).
if(CLANG_TIDY_EXE)
    # Enable compile commands database generation automatically when clang-tidy is available and we are the top-level project
    if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
        set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Enable compile commands export for clang-tidy")
    endif()

    if(BUILD_TESTING)
        # Glob test files for linting
        file(GLOB_RECURSE LINT_SOURCES CONFIGURE_DEPENDS
            "tests/*.cpp"
        )
        # Glob header files for direct standalone linting
        file(GLOB_RECURSE LINT_HEADERS CONFIGURE_DEPENDS
            "include/*.hpp"
            "include/*.h"
        )

        # Target to lint test files (driven by compile database and tests/ rules)
        add_custom_target(lint-tests
            COMMAND ${CLANG_TIDY_EXE} -p ${CMAKE_CURRENT_BINARY_DIR} ${LINT_SOURCES}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Running static analysis on tests..."
        )

        # Target to lint header files directly (driven by root rules and explicit compiler flags)
        add_custom_target(lint-headers
            COMMAND ${CLANG_TIDY_EXE} ${LINT_HEADERS} -- -std=c++20 -I${CMAKE_CURRENT_SOURCE_DIR}/include
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Running static analysis on headers..."
        )

        # Aggregate lint target combining both
        add_custom_target(lint
            DEPENDS lint-tests lint-headers
        )
    else()
        # Fallback if testing (and thus compilation commands database) is disabled
        add_custom_target(lint
            COMMAND ${CMAKE_COMMAND} -E echo "Error: lint target requires BUILD_TESTING to be enabled."
            COMMAND ${CMAKE_COMMAND} -E false
            COMMENT "Failed: BUILD_TESTING is disabled"
        )
    endif()
else()
    # Fallback if clang-tidy is missing
    add_custom_target(lint
        COMMAND ${CMAKE_COMMAND} -E echo "Error: clang-tidy was not found in PATH."
        COMMAND ${CMAKE_COMMAND} -E false
        COMMENT "Failed: clang-tidy missing"
    )
endif()

