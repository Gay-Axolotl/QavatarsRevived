# include some defines automatically made by qpm
include(qpm_defines.cmake)

cmake_minimum_required(VERSION 3.22)
project(${COMPILE_ID})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED 20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SHARED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shared)

# Compile options confirmed from a real, working, confirmed-1.40.8 mod
# (hardcpp/QBeatSaberPlus-GameTweaker) -- -fdeclspec and -fvisibility=hidden
# in particular were missing from this project's original setup and are
# very likely why bs-cordl codegen method bodies (Transform::SetParent
# etc.) weren't being emitted into our object files, causing "undefined
# hidden symbol" link errors.
add_compile_options(-frtti -fPIE -fPIC -fexceptions -fdeclspec -fvisibility=hidden -Wno-extra-qualification)
add_compile_options(-Wall -Wextra -Wno-unused-parameter)

add_compile_definitions(VERSION="${MOD_VERSION}")
add_compile_definitions(MOD_ID="${MOD_ID}")
# Also confirmed from the same reference -- UNITY_2021 and
# CORDL_RUNTIME_FIELD_NULL_CHECKS are real compile definitions that
# real, shipped bs-cordl-based mods set and this project didn't.
add_compile_definitions(UNITY_2021)
add_compile_definitions(CORDL_RUNTIME_FIELD_NULL_CHECKS)

file(GLOB_RECURSE cpp_src ${SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE c_src ${SOURCE_DIR}/*.c)

add_library(${COMPILE_ID} SHARED ${cpp_src} ${c_src})

target_include_directories(${COMPILE_ID} PRIVATE ${SOURCE_DIR})
target_include_directories(${COMPILE_ID} PRIVATE ${INCLUDE_DIR})
target_include_directories(${COMPILE_ID} PUBLIC ${SHARED_DIR})

target_link_libraries(${COMPILE_ID} PRIVATE -llog)

# QPM-generated: configures every dependency's include paths (including
# bs-cordl's nested include/ subfolder) and link libraries. Must come after
# add_library() creates the ${COMPILE_ID} target above.
include(extern.cmake)

# kaleb's asset-embedding CMake module (from extern/, once `qpm restore` has
# run) turns files under assets/ (currently just the mToon shader bundle)
# into a linkable static lib exposing them via include/assets.hpp.
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/extern/includes/kaleb/shared/cmake/assets.cmake)
    include(${CMAKE_CURRENT_SOURCE_DIR}/extern/includes/kaleb/shared/cmake/assets.cmake)
    add_assets(assets_${COMPILE_ID} STATIC ${CMAKE_CURRENT_SOURCE_DIR}/assets ${CMAKE_CURRENT_SOURCE_DIR}/include/assets.hpp)
    target_link_libraries(${COMPILE_ID} PRIVATE assets_${COMPILE_ID})
endif()
