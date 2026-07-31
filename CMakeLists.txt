cmake_minimum_required(VERSION 3.21)
project(QavatarsRevived)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(VERSION=\"${MOD_VERSION}\")
add_compile_definitions(MOD_ID=\"QavatarsRevived\")

# extra warnings, but don't fail the build on them yet -- fresh codebase, expect churn
add_compile_options(-Wall -Wextra -Wno-unused-parameter -fexceptions -frtti)

file(GLOB_RECURSE cpp_src ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE c_src ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)

add_library(${PROJECT_NAME} SHARED ${cpp_src} ${c_src})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/shared
)

# QPM auto-generates these two files at `qpm restore` time. qpm_defines.cmake
# sets up NDK/build variables; extern.cmake configures each dependency's
# include paths (including per-package compileOptions like bs-cordl's
# nested include/ subfolder) by calling target_include_directories/etc.
# directly on the "${PROJECT_NAME}" target -- so these MUST be included
# after add_library() creates that target, not before. Including them too
# early is what caused "Cannot specify include directories for target
# ... which is not built by this project" on the previous attempt.
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/qpm_defines.cmake)
    include(${CMAKE_CURRENT_SOURCE_DIR}/qpm_defines.cmake)
endif()
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/extern.cmake)
    include(${CMAKE_CURRENT_SOURCE_DIR}/extern.cmake)
endif()

# Fallback/supplement: still add the blanket extern/includes path too, for
# any dependency that doesn't need special per-package include handling.
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/extern/includes)
    target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/extern/includes)
endif()
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/extern/libs)
    target_link_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/extern/libs)
endif()

# kaleb's asset-embedding CMake module (from extern/, once `qpm restore` has
# run) turns files under assets/ (currently just the mToon shader bundle)
# into a linkable static lib exposing them via include/assets.hpp.
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/extern/includes/kaleb/shared/cmake/assets.cmake)
    include(${CMAKE_CURRENT_SOURCE_DIR}/extern/includes/kaleb/shared/cmake/assets.cmake)
    add_assets(assets_${PROJECT_NAME} STATIC ${CMAKE_CURRENT_SOURCE_DIR}/assets ${CMAKE_CURRENT_SOURCE_DIR}/include/assets.hpp)
    target_link_libraries(${PROJECT_NAME} PRIVATE assets_${PROJECT_NAME})
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE log)
