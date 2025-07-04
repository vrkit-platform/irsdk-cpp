
FUNCTION(ADD_EXAMPLE exampleDir)

  GET_FILENAME_COMPONENT(exampleName ${exampleDir} NAME)
  SET(exampleTarget ${PROJECT_NAME}_example_${exampleName})

  GET_FILENAME_COMPONENT(examplesDir ${exampleDir} DIRECTORY)
  SET(examplesCommonDir ${examplesDir}/common)

  FILE(GLOB sourceFiles ${exampleDir}/*.cpp ${examplesCommonDir}/*.cpp)
  FILE(GLOB headerFiles ${exampleDir}/*.h ${examplesCommonDir}/*.h)

  ADD_EXECUTABLE(${exampleTarget} ${sourceFiles} ${headerFiles})

  TARGET_INCLUDE_DIRECTORIES(${exampleTarget}
    PRIVATE
    ${examplesCommonDir}
  )
  TARGET_LINK_LIBRARIES(${exampleTarget} PRIVATE
    ${sdkTarget}
    ${DEP_MAGICENUM}
    ${DEP_FMT}
  )

ENDFUNCTION()