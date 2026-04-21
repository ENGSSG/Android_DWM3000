#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "qmath" for configuration "Debug"
set_property(TARGET qmath APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(qmath PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/arm-cortex-m4-hard_floating/qmath/libqmath.a"
  )

list(APPEND _cmake_import_check_targets qmath )
list(APPEND _cmake_import_check_files_for_qmath "${_IMPORT_PREFIX}/lib/arm-cortex-m4-hard_floating/qmath/libqmath.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
