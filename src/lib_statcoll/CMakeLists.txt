
add_library(sc INTERFACE)

target_compile_features(sc INTERFACE cxx_std_20)
target_include_directories(sc INTERFACE include)

FetchContent_Declare(json
    GIT_REPOSITORY https://github.com/nlohmann/json
)

FetchContent_MakeAvailable(json)

target_link_libraries(sc INTERFACE nlohmann_json::nlohmann_json)

# TODO: unit tests