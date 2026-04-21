#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "SDK" for configuration "Debug"
set_property(TARGET SDK APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(SDK PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "ASM;C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libSDK.a"
  )

list(APPEND _cmake_import_check_targets SDK )
list(APPEND _cmake_import_check_files_for_SDK "${_IMPORT_PREFIX}/lib/libSDK.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
