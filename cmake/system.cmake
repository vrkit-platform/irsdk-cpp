set(
  SYSTEM_LIBRARIES
#  advapi32
#  Comctl32
#  Dbghelp
#  Shcore
#  Shell32
#  User32
  Winmm
)

foreach(LIBRARY ${SYSTEM_LIBRARIES})
  add_library("System::${LIBRARY}" INTERFACE IMPORTED GLOBAL)
  set_property(
    TARGET "System::${LIBRARY}"
    PROPERTY IMPORTED_LIBNAME "${LIBRARY}"
  )
endforeach()