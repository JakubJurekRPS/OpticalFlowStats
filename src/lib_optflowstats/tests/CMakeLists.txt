# Testing library
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(utests_optflowstats utests.cpp)

target_compile_features(utests_optflowstats PRIVATE cxx_std_14)

target_link_libraries(utests_optflowstats PRIVATE GTest::gtest_main 
                             PRIVATE ofs)

include(GoogleTest)
gtest_discover_tests(utests_optflowstats)

enable_testing()

add_test(NAME StatsCalculation COMMAND utests_optflowstats)