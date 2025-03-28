CMAKE_MINIMUM_REQUIRED(VERSION 3.21.1)

# Windows SDK/Kit
SET(WINDOWS_SDK_VERSION "10.0.22621.0")

if(POLICY CMP0091)
  cmake_policy(SET CMP0091 NEW)
endif()

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
STRING(REPLACE "." ";" versionParts ${sdkVersion})
LIST(GET versionParts 0 sdkVersionMajor)
LIST(GET versionParts 1 sdkVersionMinor)
LIST(GET versionParts 2 sdkVersionPatch)

PROJECT(${sdkNamePrefix} VERSION ${sdkVersion} LANGUAGES CXX)

SET(IRSDKCPP_VERSION ${PROJECT_VERSION})

include(CMakePackageConfigHelpers)
include(CMakeDependentOption)
include(CheckCXXCompilerFlag)
include(GNUInstallDirs)

option(IRSDKCPP_ENABLE_PIC "Use Position-Independent Code " ON)
OPTION(IRSDKCPP_BUILD_DOCS "Generate Doxygen Documentation" OFF)
OPTION(IRSDKCPP_BUILD_TESTS "Build tests" OFF)
OPTION(IRSDKCPP_BUILD_EXAMPLES "Build examples" OFF)
OPTION(IRSDKCPP_BUILD_SHARED "Build shared libs" OFF)

cmake_dependent_option(IRSDKCPP_MSVC_SHARED_RT
  "MSVC: Build irsdkcpp with shared runtime libs (/MD)" ON
  "CMAKE_SYSTEM_NAME MATCHES Windows" OFF)

set(IRSDKCPP_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
  CACHE STRING "Path to install the CMake package to")

IF(IRSDKCPP_BUILD_SHARED)
  SET(IRSDKCPP_BUILD_STATIC OFF)    
  SET(BUILD_SHARED_LIBS ON)
  SET(BUILD_STATIC_LIBS OFF)
  SET(LIB_TYPE shared)
ELSE()
  SET(IRSDKCPP_BUILD_STATIC ON)  
  SET(BUILD_STATIC_LIBS ON)
  SET(BUILD_SHARED_LIBS OFF)
  SET(LIB_TYPE static)
ENDIF()

STRING(TOUPPER ${LIB_TYPE} IRSDKCPP_LIB_TYPE)

set(buildShared $<BOOL:${IRSDKCPP_BUILD_SHARED}>)
set(buildWindowsDLL $<AND:$<BOOL:${CMAKE_HOST_WIN32}>,${buildShared}>)
set(notMSVC $<NOT:$<CXX_COMPILER_ID:MSVC>>)
set(isMSVCSharedRT $<BOOL:${IRSDKCPP_MSVC_SHARED_RT}>)

if (NOT DEFINED CMAKE_MSVC_RUNTIME_LIBRARY)
  set(CMAKE_MSVC_RUNTIME_LIBRARY
    MultiThreaded$<$<CONFIG:Debug>:Debug>$<${isMSVCSharedRT}:DLL>)
endif()

# IRSDKCPP
SET(sdkTarget ${PROJECT_NAME}_${LIB_TYPE})

# Setup
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)
SET(CMAKE_CXX_STANDARD 23)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

# DOXYGEN
IF(IRSDKCPP_BUILD_DOCS)
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
SET(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Add WCHAR def for `spdlog` header inclusion
ADD_COMPILE_DEFINITIONS(SPDLOG_WCHAR_TO_UTF8_SUPPORT SPDLOG_WCHAR_SUPPORT SPDLOG_NO_EXCEPTIONS)

# Include CMake helper libs
INCLUDE(cmake/targets.cmake)
INCLUDE(cmake/fs.cmake)
INCLUDE(cmake/dependencies.cmake NO_POLICY_SCOPE)
INCLUDE(cmake/example_target.cmake)

# Include test tools
IF(IRSDKCPP_BUILD_TESTS)
  ENABLE_TESTING()
  INCLUDE(CTest)
  INCLUDE(GoogleTest)
ENDIF()

# SDK Include dir
SET(sdkPrivateIncludeDir ${CMAKE_SOURCE_DIR}/packages/sdk/src)
SET(sdkIncludeDir ${CMAKE_SOURCE_DIR}/packages/sdk/include)

ADD_SUBDIRECTORY(packages)


