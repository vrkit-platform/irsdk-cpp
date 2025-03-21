
FUNCTION(ADD_EXAMPLE exampleDir)
  IF(IRSDKCPP_BUILD_STATIC)
    GET_FILENAME_COMPONENT(exampleName ${exampleDir} NAME)
    SET(exampleTarget ${PROJECT_NAME}_example_${exampleName})

    FILE(GLOB sourceFiles ${exampleDir}/*.cpp)
    FILE(GLOB headerFiles ${exampleDir}/*.h)

    ADD_EXECUTABLE(${exampleTarget} ${sourceFiles} ${headerFiles})

    TARGET_INCLUDE_DIRECTORIES(${exampleTarget}
      BEFORE
      PRIVATE
      ${sdkIncludeDir}
    )
    TARGET_LINK_LIBRARIES(${exampleTarget} PRIVATE
      ${sdkTargetStatic}
      ${DEP_MAGICENUM}
      ${DEP_FMT}
    )
  ENDIF()
ENDFUNCTION()