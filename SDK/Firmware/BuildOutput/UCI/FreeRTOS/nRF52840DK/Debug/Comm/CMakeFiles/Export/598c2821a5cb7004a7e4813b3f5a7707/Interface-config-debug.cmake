#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Interface" for configuration "Debug"
set_property(TARGET Interface APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(Interface PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libInterface.a"
  )

list(APPEND _cmake_import_check_targets Interface )
list(APPEND _cmake_import_check_files_for_Interface "${_IMPORT_PREFIX}/lib/libInterface.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
