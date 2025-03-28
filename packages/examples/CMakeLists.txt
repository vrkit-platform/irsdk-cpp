
LIST_SUBDIRS(exampleDirs ${CMAKE_CURRENT_LIST_DIR})

FOREACH(exampleDir ${exampleDirs})
  IF(EXISTS ${exampleDir}/CMakeLists.txt)
    MESSAGE(STATUS "Example (${exampleDir}) has CMakeLists.txt, including directly")
    ADD_SUBDIRECTORY(${exampleDir})
    continue()
  ELSEIF(NOT EXISTS ${exampleDir}/.enable)
    MESSAGE(VERBOSE "Example (${exampleDir}) is missing CMakeLists.txt")
    continue()
  ENDIF()

  MESSAGE(STATUS "Adding example: ${exampleDir}")
  ADD_EXAMPLE(${exampleDir})
ENDFOREACH()