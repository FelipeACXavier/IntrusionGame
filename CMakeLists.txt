cmake_minimum_required(VERSION 3.5.1)

# Define project name
project(intrusion_game VERSION 1.0)

set(ARGUMENTUM_BUILD_STATIC_LIBS ON)
add_subdirectory(src_cpp/argumentum)

# Find required packages
find_package(SDL2 REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(nlohmann_json REQUIRED)
pkg_check_modules(SDL2_ttf REQUIRED sdl2)

include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${SDL2TTF_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SOURCES "src_cpp/*.h" "src_cpp/*.cpp")
add_executable(game ${SOURCES} src_cpp/main.cpp)

# For parsing json
target_link_libraries(game nlohmann_json::nlohmann_json)

# For parsing command line
target_link_libraries(game Argumentum::argumentum)

# For SDL
target_link_libraries(game ${SDL2_LIBRARIES})
target_link_libraries(game SDL2_ttf)
