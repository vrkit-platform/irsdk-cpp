INCLUDE(GenerateExportHeader)
INCLUDE(GNUInstallDirs)
INCLUDE(CMakeParseArguments)

MACRO(SETUP_TARGET_COMPILE_DEFS targetName)
#  IF(MSVC)
#    TARGET_LINK_OPTIONS(${targetName}
#      PRIVATE
##      $<$<CONFIG:Debug>:/DEBUG:FASTLINK>
#      /NODEFAULTLIB:ucrt$<$<CONFIG:Debug>:d>.lib # include the dynamic UCRT
#      /NODEFAULTLIB:libucrt$<$<CONFIG:Debug>:d>.lib # ignore the static UCRT
#
#    )
#  ENDIF()

  TARGET_COMPILE_DEFINITIONS(
    ${targetName}
    PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
    BUILD_TYPE="${CMAKE_BUILD_TYPE}"
    FMT_USE_STRING_VIEW
    SPDLOG_NO_EXCEPTIONS=1
    SPDLOG_WCHAR_SUPPORT=1
    SPDLOG_WCHAR_TO_UTF8_SUPPORT=1
  )
ENDMACRO()

MACRO(SETUP_LIB_EXPORTS)
  SET(options NO_SOURCE_HEADERS)
  CMAKE_PARSE_ARGUMENTS(SETUP_LIB_EXPORTS "${options}" "" "" ${ARGN})

  SET(idx 0)
  SET(reqArgList sharedTarget staticTarget libSourcesVarName libHeadersVarName libHeadersPublicVarName)
  SET(argList ${SETUP_LIB_EXPORTS_UNPARSED_ARGUMENTS})
  LIST(LENGTH reqArgList reqArgCount)
  LIST(LENGTH argList argCount)
  MESSAGE(NOTICE "reqArgCount=${reqArgCount}, argCount${argCount}")
  IF(NOT ${reqArgCount} EQUAL ${argCount})
    MESSAGE(FATAL_ERROR "reqArgCount=${reqArgCount} != argCount${argCount}")
  ENDIF()
  FOREACH(argName ${reqArgList})
    LIST(GET argList ${idx} argValue)
    MATH(EXPR idx "${idx} + 1")
    MESSAGE(NOTICE "Setting ${argName} to ${argValue}")
    SET(${argName} ${argValue})
  ENDFOREACH()

  SET(targets ${sharedTarget} ${staticTarget})
  SET(macroBaseName ${sharedTarget}_and_static)
  STRING(TOUPPER ${macroBaseName} macroCompileFlag)
  SET(exportHeader ${CMAKE_CURRENT_BINARY_DIR}/${macroBaseName}_export.h)

  IF(IRSDKCPP_BUILD_SHARED)
    ADD_LIBRARY(${sharedTarget} SHARED ${${libSourcesVarName}} ${${libHeadersVarName}} ${exportHeader})
    GENERATE_EXPORT_HEADER(${sharedTarget} BASE_NAME ${macroBaseName})
    SET_TARGET_PROPERTIES(${sharedTarget} PROPERTIES
      COMPILE_FLAGS -D${macroCompileFlag})

    TARGET_SOURCES(
      ${sharedTarget}
      PUBLIC
      FILE_SET publicHeaders
      TYPE HEADERS
      BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
      FILES
      ${${libHeadersPublicVarName}}
    )

    INSTALL(TARGETS ${sharedTarget}
      EXPORT irsdkcppTargets
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
      INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
      FILE_SET publicHeaders
    )

  ENDIF()

  IF(IRSDKCPP_BUILD_STATIC)
    ADD_LIBRARY(${staticTarget} STATIC ${${libSourcesVarName}} ${${libHeadersVarName}} ${exportHeader})
    GENERATE_EXPORT_HEADER(${staticTarget} BASE_NAME ${macroBaseName})
    SET_TARGET_PROPERTIES(${staticTarget} PROPERTIES
      COMPILE_FLAGS -D${macroCompileFlag})
    target_link_options(${staticTarget} PRIVATE /NODEFAULTLIB:msvcrt$<$<CONFIG:Debug>:d>.lib)
    TARGET_SOURCES(
      ${staticTarget}
      PUBLIC
      FILE_SET publicHeaders
      TYPE HEADERS
      BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
      FILES
      ${${libHeadersPublicVarName}}
    )

    INSTALL(TARGETS ${staticTarget}
      EXPORT irsdkcppTargets
      ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
      FILE_SET publicHeaders
    )
  ENDIF()
ENDMACRO()

FUNCTION(IRSDK_CPP_CONFIGURE_TARGET_RUNTIME_STATIC TARGET)
  SET_PROPERTY(TARGET ${TARGET} PROPERTY
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
ENDFUNCTION()

FUNCTION(IRSDK_CPP_CONFIGURE_TARGET_RUNTIME_DYNAMIC TARGET)
  SET_PROPERTY(TARGET ${TARGET} PROPERTY
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
ENDFUNCTION()

FUNCTION(IRSDK_CPP_CONFIGURE_TARGET TARGET)
  SETUP_TARGET_COMPILE_DEFS(${TARGET})


ENDFUNCTION()