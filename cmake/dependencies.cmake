# System Dependencies
set(
  SYSTEM_LIBRARIES
  Winmm
)

# Generate libraries from system library list
foreach(LIBRARY ${SYSTEM_LIBRARIES})
  add_library("System::${LIBRARY}" INTERFACE IMPORTED GLOBAL)
  set_property(
    TARGET "System::${LIBRARY}"
    PROPERTY IMPORTED_LIBNAME "${LIBRARY}"
  )
endforeach()

# VCPKG Dependencies
set(DEP_PACKAGES
  Microsoft.GSL
  fmt
  spdlog
  yaml-cpp
  magic_enum
  nlohmann_json)

foreach(depPkgName ${DEP_PACKAGES})
  find_package(${depPkgName} CONFIG REQUIRED)
endforeach()


# Create Similar Aliases for each Dependency
set(DEP_YAML yaml-cpp::yaml-cpp)
set(DEP_JSON nlohmann_json::nlohmann_json)
set(DEP_MAGICENUM magic_enum::magic_enum)
set(DEP_GSL Microsoft.GSL::GSL)
set(DEP_LOG spdlog::spdlog)
set(DEP_SYS_WINMM System::Winmm)
set(DEP_FMT fmt::fmt)
set(DEP_LOG spdlog::spdlog ${DEP_FMT})

set(ALL_APP_DEPS
  ${DEP_JSON}
  ${DEP_MAGICENUM}
  ${DEP_FMT}
  ${DEP_LOG}
  ${DEP_YAML}
  ${DEP_GSL}
  ${DEP_SYS_WINMM}
)

set(ALL_SDK_DEPS
  ${DEP_MAGICENUM}
  ${DEP_GSL}
  ${DEP_FMT}
  ${DEP_YAML}
  ${DEP_LOG}
  ${DEP_SYS_WINMM}
)

function(IRSDKCPP_TARGET_LINK_SDK_LIBS TARGET)
  target_link_libraries(${TARGET} PUBLIC ${ALL_SDK_DEPS})
endfunction()

function(IRSDK_CPP_CONFIGURE_APP_LIBS TARGET)
  target_link_libraries(${TARGET} PRIVATE ${ALL_APP_DEPS})
endfunction()


if(IRSDKCPP_BUILD_TESTS)
  FIND_PACKAGE(GTest CONFIG REQUIRED)
  set(DEP_GTEST_MAIN GTest::gtest GTest::gtest_main GTest::gmock)
  set(DEP_GTEST GTest::gtest GTest::gmock)

  function(IRSDK_CPP_CONFIGURE_TESTS_EXE TARGET)
    target_link_libraries(${TARGET} ${ALL_APP_DEPS} ${DEP_GTEST_MAIN})
  endfunction()
endif()
