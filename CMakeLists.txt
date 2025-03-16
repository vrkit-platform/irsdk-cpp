cmake_minimum_required(VERSION 3.21.1)

# Windows SDK/Kit
set(WINDOWS_SDK_VERSION "10.0.22621.0")

cmake_policy(SET CMP0091 NEW)
cmake_policy(SET CMP0042 NEW)

set(sdkNamePrefix irsdkcpp)
set(sdkVersionMajor 1)
set(sdkVersionMinor 0)
set(sdkVersionPatch 0)
set(sdkVersion ${sdkVersionMajor}.${sdkVersionMinor}.${sdkVersionPatch})

project(${sdkNamePrefix} VERSION ${sdkVersion})

if(POLICY CMP0091)
  cmake_policy(SET CMP0091 NEW)
endif()

# IRSDKCPP
set(sdkTarget ${PROJECT_NAME})
set(sdkTargetStatic ${sdkTarget}_static)

# Setup
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# DOXYGEN
find_package(Doxygen REQUIRED)

# CCACHE
find_program(CCACHE_PROGRAM ccache)
if (NOT CCACHE_PROGRAM)
  find_program(CCACHE_PROGRAM ccache PATHS "$ENV{HOME}\\AppData\\Local\\Microsoft\\WinGet\\Links")
endif()

if (CCACHE_PROGRAM)
  message(NOTICE "Selected CCache: ${CCACHE_PROGRAM}")
  set(CCACHE_CMDLINE ${CCACHE_PROGRAM} base_dir=${PROJECT_SOURCE_DIR} hash_dir=false log_file=${PROJECT_SOURCE_DIR}/tmp_ccache.log)
  set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_CMDLINE})
  set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_CMDLINE})
else ()
  message(WARNING "Could not find ccache, skipping!")
endif ()

set(CMAKE_CXX_VISIBILITY_PRESET visible)

# Add WCHAR def for `spdlog` header inclusion
add_compile_definitions(SPDLOG_WCHAR_TO_UTF8_SUPPORT SPDLOG_WCHAR_SUPPORT SPDLOG_NO_EXCEPTIONS)

# Include CMake helper libs
include(cmake/system.cmake)
include(cmake/targets.cmake)
include(cmake/fs.cmake)
include(cmake/dependencies.cmake NO_POLICY_SCOPE)
include(cmake/example_target.cmake)
include(cmake/hybrid_crt.cmake NO_POLICY_SCOPE)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Include test tools
enable_testing()
include(GoogleTest)
# SDK Include dir
set(sdkIncludeDir ${CMAKE_SOURCE_DIR}/packages/sdk/include)

add_subdirectory(packages)


