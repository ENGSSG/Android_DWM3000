#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Helpers" for configuration "Release"
set_property(TARGET Helpers APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(Helpers PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libHelpers.a"
  )

list(APPEND _cmake_import_check_targets Helpers )
list(APPEND _cmake_import_check_files_for_Helpers "${_IMPORT_PREFIX}/lib/libHelpers.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
