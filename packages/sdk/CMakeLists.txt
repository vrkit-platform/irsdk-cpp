
# SDK Source & headers
FILE(GLOB_RECURSE sdkTestSources tests/*.cpp)
FILE(GLOB_RECURSE sdkSources src/*.cpp)
FILE(GLOB_RECURSE sdkHeaders include/*.h)
FILE(GLOB_RECURSE sdkInternalHeaders src/*.h)

# Create libraries
SETUP_LIB_EXPORTS(${sdkTarget} ${sdkTargetStatic} sdkSources sdkInternalHeaders sdkHeaders)

IF(IRSDKCPP_BUILD_SHARED)
  IRSDK_CPP_CONFIGURE_TARGET(${sdkTarget})
  IRSDK_CPP_CONFIGURE_SDK_LIBS(${sdkTarget})
ENDIF()

IF(IRSDKCPP_BUILD_STATIC)
  IRSDK_CPP_CONFIGURE_TARGET(${sdkTargetStatic})
  IRSDK_CPP_CONFIGURE_SDK_LIBS(${sdkTargetStatic})
ENDIF()

# Export targets for use by find_package
INSTALL(EXPORT irsdkcppTargets
  FILE irsdkcppTargets.cmake
  NAMESPACE irsdkcpp::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/irsdkcpp)

# Install a cmake configuration file
INCLUDE(CMakePackageConfigHelpers)

# Generate and install the configuration file
CONFIGURE_PACKAGE_CONFIG_FILE(
  ${CMAKE_CURRENT_LIST_DIR}/Config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/irsdkcppConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/irsdkcpp)

# Generate and install the version file
WRITE_BASIC_PACKAGE_VERSION_FILE(
  ${CMAKE_CURRENT_BINARY_DIR}/irsdkcppConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion)

INSTALL(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/irsdkcppConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/irsdkcppConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/irsdkcpp)

# Documentation Generation
IF(BUILD_DOCS)
  SET(DOXYGEN_PROJECT_NAME irsdkcpp)
  SET(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/docs/xml)
  SET(DOXYGEN_RECURSIVE YES)
  SET(DOXYGEN_XML_OUTPUT YES)

  DOXYGEN_ADD_DOCS(
    irsdkcpp_docs
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/include
  )
ENDIF()

IF(BUILD_TESTS)
  IF(IRSDKCPP_BUILD_STATIC)
    SET(testTargetName ${sdkTarget}_tests_static)
    ADD_EXECUTABLE(${testTargetName} ${sdkTestSources})

    IRSDK_CPP_CONFIGURE_TARGET(
      ${testTargetName}
    )

    TARGET_LINK_LIBRARIES(${testTargetName}
      ${sdkTargetStatic}
      GTest::gtest GTest::gmock
    )

    GTEST_ADD_TESTS(
      TARGET ${testTargetName}
      SOURCES ${sdkTestSources}
      TEST_PREFIX static
    )
  ENDIF()

  IF(IRSDKCPP_BUILD_SHARED)
    SET(testTargetName ${sdkTarget}_tests_shared)
    ADD_EXECUTABLE(${testTargetName} ${sdkTestSources})
    TARGET_LINK_LIBRARIES(${testTargetName}
      ${sdkTarget}
      GTest::gtest GTest::gmock
    )

    GTEST_ADD_TESTS(
      TARGET ${testTargetName}
      SOURCES ${sdkTestSources}
      TEST_PREFIX shared
    )
  ENDIF()

ENDIF()
