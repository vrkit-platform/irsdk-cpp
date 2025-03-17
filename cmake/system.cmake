set(
  SYSTEM_LIBRARIES
  Winmm
)

foreach(LIBRARY ${SYSTEM_LIBRARIES})
  add_library("System::${LIBRARY}" INTERFACE IMPORTED GLOBAL)
  set_property(
    TARGET "System::${LIBRARY}"
    PROPERTY IMPORTED_LIBNAME "${LIBRARY}"
  )
endforeach()