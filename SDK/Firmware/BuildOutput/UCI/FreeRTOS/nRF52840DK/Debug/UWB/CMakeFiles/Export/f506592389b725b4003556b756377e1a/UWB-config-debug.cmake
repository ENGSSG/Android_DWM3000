#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "UWB" for configuration "Debug"
set_property(TARGET UWB APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(UWB PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libUWB.a"
  )

list(APPEND _cmake_import_check_targets UWB )
list(APPEND _cmake_import_check_files_for_UWB "${_IMPORT_PREFIX}/lib/libUWB.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
