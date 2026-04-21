#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "ProjectDefinition" for configuration "Release"
set_property(TARGET ProjectDefinition APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(ProjectDefinition PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libProjectDefinition.a"
  )

list(APPEND _cmake_import_check_targets ProjectDefinition )
list(APPEND _cmake_import_check_files_for_ProjectDefinition "${_IMPORT_PREFIX}/lib/libProjectDefinition.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
