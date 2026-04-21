#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "app_common" for configuration "Release"
set_property(TARGET app_common APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(app_common PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libapp_common.a"
  )

list(APPEND _cmake_import_check_targets app_common )
list(APPEND _cmake_import_check_files_for_app_common "${_IMPORT_PREFIX}/lib/libapp_common.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
