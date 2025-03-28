
FUNCTION(ADD_EXAMPLE exampleDir)

  GET_FILENAME_COMPONENT(exampleName ${exampleDir} NAME)
  SET(exampleTarget ${PROJECT_NAME}_example_${exampleName})

  FILE(GLOB sourceFiles ${exampleDir}/*.cpp)
  FILE(GLOB headerFiles ${exampleDir}/*.h)

  ADD_EXECUTABLE(${exampleTarget} ${sourceFiles} ${headerFiles})

  TARGET_LINK_LIBRARIES(${exampleTarget} PRIVATE
    ${sdkTarget}
    ${DEP_MAGICENUM}
    ${DEP_FMT}
  )

ENDFUNCTION()