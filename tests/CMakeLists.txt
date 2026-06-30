include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.5.2
)
FetchContent_MakeAvailable(Catch2)

# Default (Balanced) test target
add_executable(kaw_random_tests RandomNumbersTest.cpp)
target_link_libraries(kaw_random_tests PRIVATE kaw_random Catch2::Catch2WithMain)

# Basic seeding test target
add_executable(kaw_random_tests_basic RandomNumbersTest.cpp)
target_compile_definitions(kaw_random_tests_basic PRIVATE KAW_RANDOM_SEED_BASIC)
target_link_libraries(kaw_random_tests_basic PRIVATE kaw_random Catch2::Catch2WithMain)

# Full seeding test target
add_executable(kaw_random_tests_full RandomNumbersTest.cpp)
target_compile_definitions(kaw_random_tests_full PRIVATE KAW_RANDOM_SEED_FULL)
target_link_libraries(kaw_random_tests_full PRIVATE kaw_random Catch2::Catch2WithMain)

enable_testing()
add_test(NAME kaw_random_tests COMMAND kaw_random_tests)
add_test(NAME kaw_random_tests_basic COMMAND kaw_random_tests_basic)
add_test(NAME kaw_random_tests_full COMMAND kaw_random_tests_full)
