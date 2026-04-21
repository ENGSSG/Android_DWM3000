#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "EventManager" for configuration "Debug"
set_property(TARGET EventManager APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(EventManager PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libEventManager.a"
  )

list(APPEND _cmake_import_check_targets EventManager )
list(APPEND _cmake_import_check_files_for_EventManager "${_IMPORT_PREFIX}/lib/libEventManager.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
