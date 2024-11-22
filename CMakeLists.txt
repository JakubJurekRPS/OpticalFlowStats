cmake_minimum_required(VERSION 3.22)

project(FlowAnalyzerProject VERSION 1.0
                  DESCRIPTION "Optical flow histogram statistics generator"
                  LANGUAGES CXX)

include(FetchContent)

add_subdirectory(apps)
add_subdirectory(src)

if(BUILD_TESTING)
    include(CTest)
    enable_testing()
endif()
