
MACRO(LIST_SUBDIRS result parentDir)
  FILE(GLOB childDirs RELATIVE ${parentDir} ${parentDir}/*)
  SET(dirs "")
  FOREACH(childDir ${childDirs})
    IF(IS_DIRECTORY ${parentDir}/${childDir})
      LIST(APPEND dirs "${parentDir}/${childDir}")
    ENDIF()
  ENDFOREACH()
  SET(${result} ${dirs})
ENDMACRO()

FUNCTION(CONVERT_TO_RELATIVE_PATHS files outputVar baseDir)
  MESSAGE(NOTICE "CONVERTING TO RELATIVE PATHS
    BASE: ${baseDir}
    FILES: ${files}
")
  SET(relativePaths)

  # Loop over each file in the input list
  FOREACH(file IN LISTS files)
    FILE(RELATIVE_PATH relPath "${baseDir}" "${file}")
    LIST(APPEND relativePaths "${relPath}")
  ENDFOREACH()

  MESSAGE(NOTICE "RESULT PATHS: ${relativePaths}")
  # Return the result as the output list
  SET(${outputVar} ${relativePaths} PARENT_SCOPE)
ENDFUNCTION()
