INCLUDE(GenerateExportHeader)
INCLUDE(GNUInstallDirs)
INCLUDE(CMakeParseArguments)

MACRO(SETUP_TARGET_COMPILE_DEFS targetName)
  TARGET_COMPILE_DEFINITIONS(
    ${targetName}
    PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
  )

  TARGET_COMPILE_DEFINITIONS(
    ${targetName}
    PRIVATE
    FMT_USE_STRING_VIEW
    SPDLOG_NO_EXCEPTIONS=1
    SPDLOG_WCHAR_SUPPORT=1
    SPDLOG_WCHAR_TO_UTF8_SUPPORT=1
  )
ENDMACRO()

MACRO(SETUP_DYNAMIC_TARGET_COMPILE_OPTS targetName)
  IF(MSVC)
    # set_property(TARGET ${targetName} PROPERTY
    #   MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    TARGET_LINK_OPTIONS(${targetName}
      PRIVATE
      $<$<CONFIG:Debug>:/DEBUG:FASTLINK>
    )
  ENDIF()
ENDMACRO()

MACRO(SETUP_STATIC_TARGET_COMPILE_OPTS targetName)
  IF(MSVC)
    # set_property(TARGET ${targetName} PROPERTY
    #   MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    TARGET_LINK_OPTIONS(${targetName}
      PRIVATE
      $<$<CONFIG:Debug>:/DEBUG:FASTLINK>
    )
  ENDIF()
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

  ADD_LIBRARY(${sharedTarget} SHARED ${${libSourcesVarName}} ${${libHeadersVarName}} ${exportHeader})
  ADD_LIBRARY(${staticTarget} STATIC ${${libSourcesVarName}} ${${libHeadersVarName}} ${exportHeader})

  GENERATE_EXPORT_HEADER(${sharedTarget} BASE_NAME ${macroBaseName})
  SET_TARGET_PROPERTIES(${staticTarget} PROPERTIES
    COMPILE_FLAGS -D${macroCompileFlag})

  MESSAGE(NOTICE "Creating lib targets (${targets}) with source dir: ${CMAKE_CURRENT_SOURCE_DIR}")
  FOREACH(target ${targets})
    TARGET_LINK_OPTIONS(${target} PRIVATE /NODEFAULTLIB:library)
    TARGET_SOURCES(
      ${target}
      PUBLIC
      FILE_SET publicHeaders
      TYPE HEADERS
      BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
      FILES
      ${${libHeadersPublicVarName}}
    )
  ENDFOREACH()

  INSTALL(TARGETS ${sharedTarget}
    EXPORT irsdkcppTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILE_SET publicHeaders
  )

  INSTALL(TARGETS ${staticTarget}
    EXPORT irsdkcppTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FILE_SET publicHeaders
  )

ENDMACRO()

MACRO(SETUP_MOD_EXPORTS)
  SET(options NO_SOURCE_HEADERS)
  CMAKE_PARSE_ARGUMENTS(SETUP_MOD_EXPORTS "${options}" "" "" ${ARGN})

  SET(idx 0)
  SET(reqArgList sharedTarget staticTarget libSourcesVarName libHeadersVarName libHeadersPublicVarName)
  SET(argList ${SETUP_MOD_EXPORTS_UNPARSED_ARGUMENTS})
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

  # sharedTarget staticTarget libSourcesVarName libHeadersVarName libHeadersPublicVarName
  #  set(staticTarget ${sharedTarget}_static)
  SET(targets ${sharedTarget} ${staticTarget})
  SET(macroBaseName ${sharedTarget}_and_static)
  STRING(TOUPPER ${macroBaseName} macroCompileFlag)
  SET(exportHeader ${CMAKE_CURRENT_BINARY_DIR}/${macroBaseName}_export.h)

  ADD_LIBRARY(${sharedTarget} MODULE ${${libSourcesVarName}} ${${libHeadersVarName}} ${exportHeader})
  ADD_LIBRARY(${staticTarget} MODULE ${${libSourcesVarName}} ${${libHeadersVarName}} ${exportHeader})

  GENERATE_EXPORT_HEADER(${sharedTarget} BASE_NAME ${macroBaseName})
  SET_TARGET_PROPERTIES(${staticTarget} PROPERTIES
    COMPILE_FLAGS -D${macroCompileFlag})

  MESSAGE(NOTICE "Creating lib targets (${targets}) with source dir: ${CMAKE_CURRENT_SOURCE_DIR}")
  FOREACH(target ${targets})
    TARGET_INCLUDE_DIRECTORIES(${target} PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
      $<INSTALL_INTERFACE:include>  # <prefix>/include/mylib
    )

    IF(NOT SETUP_MOD_EXPORTS_NO_SOURCE_HEADERS)
      TARGET_INCLUDE_DIRECTORIES(${target} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>  # <prefix>/include/mylib
      )

      TARGET_SOURCES(
        ${target}
        PUBLIC
        FILE_SET publicHeaders
        TYPE HEADERS
        FILES
        ${${libHeadersPublicVarName}}
      )
      #      install(TARGETS ${target} FILE_SET publicHeaders DESTINATION ${INCLUDE_INSTALL_DIR})
    ENDIF()

    #    install(TARGETS ${target} DESTINATION ${LIBRARY_INSTALL_DIR})
    #    install(TARGETS ${target} FILE_SET publicExportHeaders DESTINATION ${INCLUDE_INSTALL_DIR})
    #    install(TARGETS ${target} FILES
    #      ${exportHeader} DESTINATION ${INCLUDE_INSTALL_DIR}
    #    )
  ENDFOREACH()
ENDMACRO()

FUNCTION(IRSDK_CPP_CONFIGURE_TARGET_RUNTIME TARGET)
  SET_PROPERTY(TARGET ${TARGET} PROPERTY
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
ENDFUNCTION()

FUNCTION(IRSDK_CPP_CONFIGURE_TARGET TARGET)
  SETUP_TARGET_COMPILE_DEFS(${TARGET})

  TARGET_COMPILE_DEFINITIONS(
    ${TARGET}
    PRIVATE
    BUILD_TYPE="${CMAKE_BUILD_TYPE}"
  )
ENDFUNCTION()