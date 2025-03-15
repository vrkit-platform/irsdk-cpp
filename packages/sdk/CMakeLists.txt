
# SDK Source & headers
FILE(GLOB_RECURSE sdkTestSources tests/*.cpp)
FILE(GLOB_RECURSE sdkSources src/*.cpp)
FILE(GLOB_RECURSE sdkHeaders include/*.h)
FILE(GLOB_RECURSE sdkInternalHeaders src/*.h)

# Create libraries
SETUP_LIB_EXPORTS(${sdkTarget} ${sdkTargetStatic} sdkSources sdkInternalHeaders sdkHeaders)

IRSDK_CPP_CONFIGURE_TARGET(${sdkTarget})
IRSDK_CPP_CONFIGURE_TARGET(${sdkTargetStatic})

# Add dependency
IRSDK_CPP_CONFIGURE_SDK_LIBS(${sdkTarget})
IRSDK_CPP_CONFIGURE_SDK_LIBS(${sdkTargetStatic})

# Tests
ENABLE_TESTING()
INCLUDE(GoogleTest)

SET(testTargetName ${sdkTarget}_tests)
ADD_EXECUTABLE(${testTargetName} ${sdkTestSources})
TARGET_LINK_LIBRARIES(${testTargetName} PUBLIC
  ${sdkTargetStatic})
IRSDK_CPP_CONFIGURE_TESTS_EXE(${testTargetName})
GTEST_ADD_TESTS(
  TARGET ${testTargetName}
  SOURCES ${sdkTestSources}
)

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
