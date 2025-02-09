cmake_minimum_required(VERSION 3.23)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  message(STATUS "→ Library loaded as top project")

  project(
    MPRISControlProject
    DESCRIPTION "MPRIS players controller library"
    LANGUAGES CXX)

  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

  add_subdirectory(examples)
endif()

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(dbus REQUIRED IMPORTED_TARGET dbus-1)

# Add library
add_library(mprisctl)
target_sources(mprisctl PRIVATE src/mprisctl.cpp)
target_include_directories(mprisctl PUBLIC include)
target_link_libraries(mprisctl PRIVATE PkgConfig::dbus)
