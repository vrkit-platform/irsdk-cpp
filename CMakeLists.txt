CMAKE_MINIMUM_REQUIRED(VERSION 3.21.1)

# Windows SDK/Kit
SET(WINDOWS_SDK_VERSION "10.0.22621.0")

CMAKE_POLICY(SET CMP0091 NEW)
CMAKE_POLICY(SET CMP0042 NEW)

SET(sdkNamePrefix irsdkcpp)

# Read version number from a file
SET(VERSION_FILE ${CMAKE_SOURCE_DIR}/version.txt)

IF(EXISTS ${VERSION_FILE})
  FILE(READ ${VERSION_FILE} sdkVersion)
  STRING(STRIP ${sdkVersion} sdkVersion) # Strip any excess whitespaces or newlines
  MESSAGE(STATUS "SDK Version read from file: ${sdkVersion}")
ELSE()
  MESSAGE(FATAL_ERROR "Version file not found: ${VERSION_FILE}")
ENDIF()

# Parse version into major, minor, and patch
STRING(REPLACE "." ";" version_parts ${sdkVersion})
LIST(GET version_parts 0 sdkVersionMajor)
LIST(GET version_parts 1 sdkVersionMinor)
LIST(GET version_parts 2 sdkVersionPatch)

PROJECT(${sdkNamePrefix} VERSION ${sdkVersion})

OPTION(BUILD_DOCS "Generate Doxygen Documentation" OFF)
OPTION(BUILD_TESTS "Build tests" OFF)
OPTION(BUILD_EXAMPLES "Build examples" OFF)

# IRSDKCPP
SET(sdkTarget ${PROJECT_NAME})
SET(sdkTargetStatic ${sdkTarget}_static)

# Setup
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)
SET(CMAKE_CXX_STANDARD 23)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

# DOXYGEN
IF(BUILD_DOCS)
  FIND_PACKAGE(Doxygen REQUIRED)
ENDIF()

# CCACHE
FIND_PROGRAM(CCACHE_PROGRAM ccache)
IF(NOT CCACHE_PROGRAM)
  FIND_PROGRAM(CCACHE_PROGRAM ccache PATHS "$ENV{HOME}\\AppData\\Local\\Microsoft\\WinGet\\Links")
ENDIF()

IF(CCACHE_PROGRAM)
  MESSAGE(NOTICE "Selected CCache: ${CCACHE_PROGRAM}")
  SET(CCACHE_CMDLINE ${CCACHE_PROGRAM} base_dir=${PROJECT_SOURCE_DIR} hash_dir=false log_file=${PROJECT_SOURCE_DIR}/tmp_ccache.log)
  SET(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_CMDLINE})
  SET(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_CMDLINE})
ELSE()
  MESSAGE(WARNING "Could not find ccache, skipping!")
ENDIF()

SET(CMAKE_CXX_VISIBILITY_PRESET visible)

# Add WCHAR def for `spdlog` header inclusion
ADD_COMPILE_DEFINITIONS(SPDLOG_WCHAR_TO_UTF8_SUPPORT SPDLOG_WCHAR_SUPPORT SPDLOG_NO_EXCEPTIONS)

# Include CMake helper libs
INCLUDE(cmake/system.cmake)
INCLUDE(cmake/targets.cmake)
INCLUDE(cmake/fs.cmake)
INCLUDE(cmake/dependencies.cmake NO_POLICY_SCOPE)
INCLUDE(cmake/example_target.cmake)
# include(cmake/hybrid_crt.cmake NO_POLICY_SCOPE)

SET(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Include test tools
ENABLE_TESTING()
INCLUDE(GoogleTest)
# SDK Include dir
SET(sdkIncludeDir ${CMAKE_SOURCE_DIR}/packages/sdk/include)

ADD_SUBDIRECTORY(packages)


