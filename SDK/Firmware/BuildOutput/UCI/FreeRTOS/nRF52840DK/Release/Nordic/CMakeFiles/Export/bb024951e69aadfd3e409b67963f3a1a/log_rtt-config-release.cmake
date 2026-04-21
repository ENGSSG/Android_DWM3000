#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "log_rtt" for configuration "Release"
set_property(TARGET log_rtt APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(log_rtt PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/liblog_rtt.a"
  )

list(APPEND _cmake_import_check_targets log_rtt )
list(APPEND _cmake_import_check_files_for_log_rtt "${_IMPORT_PREFIX}/lib/liblog_rtt.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
