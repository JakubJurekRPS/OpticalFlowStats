# OpticalFlowStats

# TODOs:

Use streams to enable simultaneous collecting and printing/writing results to files

└─ OpticalFlowStats
   ├─ flow_analyzer.cpp
   │  └─ line 67: TODO : CUDA optical flow calculation
   ├─ predefstats.cpp
   │  └─ line 29: TODO : Implement CalcHoV operator()
   ├─ stats_definitions.cu
   │  ├─ line 9: TODO : check if it is possible to avoid bank conflicts
   │  ├─ line 109: TODO : use shared memory and warp shuffle
   │  └─ line 303: TODO : size of shared memory can be made a half of curr or even less using warp shuffle
   ├─ utests.cpp
   │  └─ line 7: TODO : other tests
   ├─ collector.hpp
   │  ├─ line 67: TODO : Default constructible functors?
   │  ├─ line 68: TODO : Other concepts for Collector template types
   │  └─ line 80: TODO : Use variant and get() instead of unique_ptrs. It's not worth to use pointers as accelaration is tiny
   └─ ut_collector.hpp
      └─ line 4: TODO : unit test for Collector

# Build options:
  -DUSE_GPU
      Use cuda-defined statistics equivalents
  -DBUILD_TESTING
      Build unit tests

# Testing
    make test -C /path/to/build/directory
