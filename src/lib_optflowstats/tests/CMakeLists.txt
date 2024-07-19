# Testing library
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(utests utests.cpp)

target_compile_features(utests PRIVATE cxx_std_14)

target_link_libraries(utests PRIVATE GTest::gtest_main 
                             PRIVATE ofs)

include(GoogleTest)
gtest_discover_tests(utests)

enable_testing()

add_test(NAME StatsCalculation COMMAND utests)