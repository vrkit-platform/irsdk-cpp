
# SDK Source & headers
FILE(GLOB_RECURSE sdkTestSources tests/*.cpp)
FILE(GLOB_RECURSE sdkSources src/*.cpp)
FILE(GLOB_RECURSE sdkHeaders include/*.h)
FILE(GLOB_RECURSE sdkInternalHeaders src/*.h)

# Create libraries
SETUP_LIB_EXPORTS(${sdkTarget} ${sdkTargetStatic} sdkSources sdkInternalHeaders sdkHeaders)
SETUP_TARGET_COMPILE_DEFS(${sdkTarget})
SETUP_TARGET_COMPILE_DEFS(${sdkTargetStatic})

#SETUP_DYNAMIC_TARGET_COMPILE_OPTS(${sdkTarget})
SETUP_STATIC_TARGET_COMPILE_OPTS(${sdkTarget})
SETUP_STATIC_TARGET_COMPILE_OPTS(${sdkTargetStatic})
#SETUP_DYNAMIC_TARGET_COMPILE_OPTS(${sdkTargetStatic})

IRSDK_CPP_CONFIGURE_TARGET(${sdkTarget})
IRSDK_CPP_CONFIGURE_TARGET(${sdkTargetStatic})

# Add dependency
IRSDK_CPP_CONFIGURE_SDK_LIBS(${sdkTarget})
IRSDK_CPP_CONFIGURE_SDK_LIBS(${sdkTargetStatic})

## Install target for dynamic library
#install(TARGETS ${sdkTarget}
#  EXPORT ${sdkTarget}Targets
#  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
#
## Install target for static library
#install(TARGETS ${sdkTargetStatic}
#  EXPORT ${sdkTarget}Targets
#  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

# Install headers
#install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/
#  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Export targets for use by find_package
install(EXPORT ${sdkTarget}Targets
  FILE ${sdkTarget}Targets.cmake
  NAMESPACE ${PROJECT_NAME}::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

# Install a cmake configuration file
include(CMakePackageConfigHelpers)

# Generate and install the configuration file
configure_package_config_file(
  ${CMAKE_CURRENT_LIST_DIR}/Config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

# Generate and install the version file
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})


# Documentation Generation
SET(DOXYGEN_PROJECT_NAME ${PROJECT_NAME})
SET(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/docs/xml)
SET(DOXYGEN_RECURSIVE YES)
SET(DOXYGEN_XML_OUTPUT YES)

DOXYGEN_ADD_DOCS(
  ${sdkTarget}_docs
  ${CMAKE_CURRENT_LIST_DIR}/src
  ${CMAKE_CURRENT_LIST_DIR}/include
)


# Tests
ENABLE_TESTING()
INCLUDE(GoogleTest)

SET(testTargetName ${sdkTarget}_tests)
ADD_EXECUTABLE(${testTargetName} ${sdkTestSources})
TARGET_LINK_LIBRARIES(${testTargetName} PUBLIC
  ${sdkTargetStatic})
IRSDK_CPP_CONFIGURE_TESTS_EXE(${testTargetName})
IRSDK_CPP_CONFIGURE_TARGET_RUNTIME(${testTargetName})

GTEST_ADD_TESTS(
  TARGET ${testTargetName}
  SOURCES ${sdkTestSources}
)

