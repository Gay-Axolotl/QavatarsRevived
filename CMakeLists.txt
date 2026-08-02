cmake_minimum_required(VERSION 3.21)
project(QavatarsRevived)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(VERSION="${MOD_VERSION}")
add_compile_definitions(MOD_ID="QavatarsRevived")

# extra warnings, but don't fail the build on them yet -- fresh codebase, expect churn
add_compile_options(-Wall -Wextra -Wno-unused-parameter -fexceptions -frtti)

file(GLOB_RECURSE cpp_src ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE c_src ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)

# IMPORTANT: QPM's generated extern.cmake hardcodes target_* calls against
# the mod's qpm.json "info.id" (here: "qavatarsrevived", lowercase) -- NOT
# CMake's PROJECT_NAME/"QavatarsRevived". Keep this in sync with qpm.json's
# info.id if that ever changes.
set(MOD_TARGET_NAME qavatarsrevived)

add_library(${MOD_TARGET_NAME} SHARED ${cpp_src} ${c_src})
set_target_properties(${MOD_TARGET_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

target_include_directories(${MOD_TARGET_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/shared
)

# QPM auto-generates these two files at `qpm restore` time and MUST be
# included after add_library() creates the target above.
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/qpm_defines.cmake)
    include(${CMAKE_CURRENT_SOURCE_DIR}/qpm_defines.cmake)
endif()
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/extern.cmake)
    include(${CMAKE_CURRENT_SOURCE_DIR}/extern.cmake)
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/extern/includes)
    target_include_directories(${MOD_TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/extern/includes)
endif()
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/extern/libs)
    target_link_directories(${MOD_TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/extern/libs)
endif()

# kaleb's asset-embedding CMake module (from extern/, once `qpm restore` has
# run) turns files under assets/ (currently just the mToon shader bundle)
# into a linkable static lib exposing them via include/assets.hpp.
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/extern/includes/kaleb/shared/cmake/assets.cmake)
    include(${CMAKE_CURRENT_SOURCE_DIR}/extern/includes/kaleb/shared/cmake/assets.cmake)
    add_assets(assets_${PROJECT_NAME} STATIC ${CMAKE_CURRENT_SOURCE_DIR}/assets ${CMAKE_CURRENT_SOURCE_DIR}/include/assets.hpp)
    target_link_libraries(${MOD_TARGET_NAME} PRIVATE assets_${PROJECT_NAME})
endif()

target_link_libraries(${MOD_TARGET_NAME} PRIVATE log)
