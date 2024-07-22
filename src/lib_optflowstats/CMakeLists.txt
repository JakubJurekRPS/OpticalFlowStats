
option(USE_GPU "Use provided gpu accelerated functions" OFF)
add_library(ofs src/predefstats.cpp)

target_compile_features(ofs PUBLIC cxx_std_20)

find_package( OpenCV REQUIRED )
target_include_directories(ofs  PUBLIC include
                                PUBLIC ${OpenCV_INCLUDE_DIRS} )

if(USE_GPU)
    message(STATUS "CUDA support enabled")
    include(CheckLanguage)
    check_language(CUDA)
    target_compile_definitions(ofs PUBLIC USE_GPU)
endif()

if(CMAKE_CUDA_COMPILER)
    
    if(NOT DEFINED CMAKE_CUDA_STANDARD)
        set(CMAKE_CUDA_STANDARD 17)
        set(CMAKE_CUDA_STANDARD_REQUIRED ON)
    endif()

    enable_language(CUDA)
    set_target_properties(ofs PROPERTIES
                            CUDA_SEPARABLE_COMPILATION ON
                            CUDA_RESOLVE_DEVICE_SYMBOLS ON)

    find_package(CUDAToolkit REQUIRED)
    target_sources(ofs PRIVATE src/stats_definitions.cu)
    target_include_directories(ofs PRIVATE ${CUDAToolkit_INCLUDE_DIRS})
    
    if(NOT DEFINED GPU_ARCH)
        message(FATAL_ERROR "GPU_ARCH not defined")
    endif()

    set_target_properties(ofs PROPERTIES CUDA_ARCHITECTURES "${GPU_ARCH}")
    target_link_libraries(ofs   PRIVATE ${CUDA_CUDART_LIBRARY}
                                PRIVATE ${CUDA_cudadevrt_LIBRARY})
                                
    set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} --use_fast_math --extended-lambda -std=c++20")
    message(STATUS "Building with CUDA")
else()
    if(USE_GPU)
        message(WARNING "CUDA support requested but not found")
    endif()
    message(STATUS "Building without CUDA")
    target_sources(ofs PRIVATE src/stats_definitions.cpp)
endif()

target_link_libraries(ofs   PUBLIC sc
                            PUBLIC ${OpenCV_LIBS})


if(BUILD_TESTING)
    add_subdirectory(tests)
    enable_testing()
endif()

