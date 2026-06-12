cmake_minimum_required(VERSION 3.15)
project(rad2d VERSION 0.2.0 LANGUAGES CXX)

# ---------------------------------------------------------------------------
# RAD-2D: a small raylib-based drawing/animation wrapper for 2D games.
#
# Consume it with either:
#   add_subdirectory(rad2d)            # vendored / submodule
#   FetchContent_Declare(...)          # fetched at configure time
# then link against the `rad2d::rad2d` target.
# ---------------------------------------------------------------------------

# Only build the example when this is the top-level project (not when another
# project pulls us in via add_subdirectory / FetchContent).
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(RAD2D_IS_TOP_LEVEL ON)
else()
    set(RAD2D_IS_TOP_LEVEL OFF)
endif()
option(RAD2D_BUILD_EXAMPLE "Build the example program" ${RAD2D_IS_TOP_LEVEL})

# --- raylib dependency -----------------------------------------------------
# Prefer raylib's installed CMake config; fall back to pkg-config.
find_package(raylib QUIET)
if(NOT raylib_FOUND)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(raylib REQUIRED IMPORTED_TARGET raylib)
    add_library(raylib ALIAS PkgConfig::raylib)
endif()

# --- the library -----------------------------------------------------------
add_library(rad2d
    animation/animation.cpp
    draw/draw.cpp
    assets/assets.cpp
    tiles/tiles.cpp
)
add_library(rad2d::rad2d ALIAS rad2d)

target_compile_features(rad2d PUBLIC cxx_std_17)
target_link_libraries(rad2d PUBLIC raylib)

# Repo root is on the include path so consumers get the umbrella header:
#   #include "rad2d.hpp"
target_include_directories(rad2d PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

# --- example ---------------------------------------------------------------
if(RAD2D_BUILD_EXAMPLE)
    add_executable(rad2d_example main.cpp)
    target_link_libraries(rad2d_example PRIVATE rad2d::rad2d)
endif()

# --- install ---------------------------------------------------------------
include(GNUInstallDirs)

install(TARGETS rad2d
    EXPORT rad2dTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Headers keep their directory layout so the relative includes between them
# (and the umbrella header) still resolve once installed.
install(FILES rad2d.hpp DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES animation/animation.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/animation)
install(FILES draw/draw.h            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/draw)
install(FILES assets/assets.h        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/assets)
install(FILES tiles/tiles.h          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tiles)

# Export targets so downstream projects can find_package(rad2d).
install(EXPORT rad2dTargets
    FILE rad2dTargets.cmake
    NAMESPACE rad2d::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/rad2d
)

# --- package config --------------------------------------------------------
# install(EXPORT ...) only writes rad2dTargets.cmake; find_package(rad2d) also
# needs a Config file (which re-resolves raylib) and a version file.
include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/rad2dConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/rad2dConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/rad2d
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/rad2dConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/rad2dConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/rad2dConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/rad2d
)
