#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "qhal" for configuration "Debug"
set_property(TARGET qhal APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(qhal PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/arm-cortex-m4-hard_floating/qhal/libqhal.a"
  )

list(APPEND _cmake_import_check_targets qhal )
list(APPEND _cmake_import_check_files_for_qhal "${_IMPORT_PREFIX}/lib/arm-cortex-m4-hard_floating/qhal/libqhal.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
