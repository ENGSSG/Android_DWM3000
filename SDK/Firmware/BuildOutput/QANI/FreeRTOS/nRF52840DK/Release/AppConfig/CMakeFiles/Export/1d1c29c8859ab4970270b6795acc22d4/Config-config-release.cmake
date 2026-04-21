#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Config" for configuration "Release"
set_property(TARGET Config APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(Config PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libConfig.a"
  )

list(APPEND _cmake_import_check_targets Config )
list(APPEND _cmake_import_check_files_for_Config "${_IMPORT_PREFIX}/lib/libConfig.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
