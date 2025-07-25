cmake_minimum_required(VERSION 3.12)
project (kate)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR
        "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options("$<$<CONFIG:DEBUG>:-Wall;-Wextra;-pedantic>")
endif()

# shared libs
set (LIBRARY_TYPE SHARED)

# search packages used by KDE
find_package(KDE4 4.8.0 REQUIRED)
include (KDE4Defaults)
include (MacroLibrary)
include (CheckFunctionExists)

# no in source builds
macro_ensure_out_of_source_build("kate requires an out of source build. Please create a separate build directory and run 'cmake path_to_kate [options]' there.")

# config.h
if(NOT APPLE)
  check_function_exists (fdatasync HAVE_FDATASYNC)
  check_function_exists (ctermid HAVE_CTERMID)
endif(NOT APPLE)

configure_file (config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

# enforce build of tests
set (KDE4_BUILD_TESTS OFF)

# KDE/QT definitions we want
add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
add_definitions (-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS -DKTEXTEDITOR_NO_DEPRECATED)

# global include directories
include_directories (
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}
  ${KDE4_INCLUDES}
  ${KDE4_KIO_INCLUDES}
)

# kdelibs 4.11 is required
if("${KDE_VERSION}" VERSION_LESS 4.10.60)
  message(FATAL_ERROR "kdelibs version ${KDE_VERSION}, but 4.11 is required. Set the option BUILD_KTEXTEDITOR to ON to build anyway.")
endif()

# kate part
macro_optional_add_subdirectory (part)

# kate application
macro_optional_add_subdirectory (kate)

# kwrite
macro_optional_add_subdirectory (kwrite)

# addons, e.g. ktexteditor plugins, kate plugins, plasma applets, ...
macro_optional_add_subdirectory (addons)

# docs
macro_optional_add_subdirectory (doc)

# tests
macro_optional_add_subdirectory (tests)

# display features ;)
macro_display_feature_log()
