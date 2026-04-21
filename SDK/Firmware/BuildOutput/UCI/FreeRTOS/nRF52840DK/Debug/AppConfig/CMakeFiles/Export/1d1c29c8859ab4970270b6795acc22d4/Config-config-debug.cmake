#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Config" for configuration "Debug"
set_property(TARGET Config APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(Config PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libConfig.a"
  )

list(APPEND _cmake_import_check_targets Config )
list(APPEND _cmake_import_check_files_for_Config "${_IMPORT_PREFIX}/lib/libConfig.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
