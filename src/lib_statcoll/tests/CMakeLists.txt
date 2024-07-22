# Testing library
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(utests_statcoll utests.cpp)

target_compile_features(utests_statcoll PRIVATE cxx_std_14)

target_link_libraries(utests_statcoll PRIVATE GTest::gtest_main 
                             PRIVATE sc)

include(GoogleTest)
gtest_discover_tests(utests_statcoll)

enable_testing()

add_test(NAME StatsCollection COMMAND utests_statcoll)