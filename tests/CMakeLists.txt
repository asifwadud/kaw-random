include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.5.2
)
FetchContent_MakeAvailable(Catch2)

add_executable(kaw_random_numbers_tests RandomNumbersTest.cpp)
target_link_libraries(kaw_random_numbers_tests PRIVATE kaw_random_numbers Catch2::Catch2WithMain)

enable_testing()
add_test(NAME kaw_random_numbers_tests COMMAND kaw_random_numbers_tests)
