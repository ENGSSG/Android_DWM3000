#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "qplatform_deca_compat" for configuration "Debug"
set_property(TARGET qplatform_deca_compat APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(qplatform_deca_compat PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/arm-cortex-m4-hard_floating/qplatform_deca_compat/libqplatform_deca_compat.a"
  )

list(APPEND _cmake_import_check_targets qplatform_deca_compat )
list(APPEND _cmake_import_check_files_for_qplatform_deca_compat "${_IMPORT_PREFIX}/lib/arm-cortex-m4-hard_floating/qplatform_deca_compat/libqplatform_deca_compat.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
