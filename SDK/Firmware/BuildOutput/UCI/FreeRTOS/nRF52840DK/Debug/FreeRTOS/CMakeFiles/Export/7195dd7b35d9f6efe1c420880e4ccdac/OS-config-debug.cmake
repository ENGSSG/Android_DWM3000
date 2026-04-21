#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "OS" for configuration "Debug"
set_property(TARGET OS APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(OS PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libOS.a"
  )

list(APPEND _cmake_import_check_targets OS )
list(APPEND _cmake_import_check_files_for_OS "${_IMPORT_PREFIX}/lib/libOS.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
