#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "reporter" for configuration "Debug"
set_property(TARGET reporter APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(reporter PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libreporter.a"
  )

list(APPEND _cmake_import_check_targets reporter )
list(APPEND _cmake_import_check_files_for_reporter "${_IMPORT_PREFIX}/lib/libreporter.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
