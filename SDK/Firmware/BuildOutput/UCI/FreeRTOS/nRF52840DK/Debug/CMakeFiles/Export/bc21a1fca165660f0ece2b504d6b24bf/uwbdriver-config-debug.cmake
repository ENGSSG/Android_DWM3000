#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "uwbdriver" for configuration "Debug"
set_property(TARGET uwbdriver APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(uwbdriver PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/arm-cortex-m4-hard_floating/uwbdriver/libuwbdriver.a"
  )

list(APPEND _cmake_import_check_targets uwbdriver )
list(APPEND _cmake_import_check_files_for_uwbdriver "${_IMPORT_PREFIX}/lib/arm-cortex-m4-hard_floating/uwbdriver/libuwbdriver.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
