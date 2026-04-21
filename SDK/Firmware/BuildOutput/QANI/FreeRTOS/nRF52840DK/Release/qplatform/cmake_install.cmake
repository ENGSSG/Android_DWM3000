# Install script for directory: /home/engssg/Android_DWM3000/SDK/Firmware/Libs/uwb-stack/libs/qplatform

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/arm-none-eabi-objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/qplatform_deca_compat" TYPE STATIC_LIBRARY OPTIONAL FILES "/home/engssg/Android_DWM3000/SDK/Firmware/BuildOutput/QANI/FreeRTOS/nRF52840DK/Release/qplatform/libqplatform_deca_compat.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/qplatform_deca_compat/cmake/qplatform_deca_compat-config.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/qplatform_deca_compat/cmake/qplatform_deca_compat-config.cmake"
         "/home/engssg/Android_DWM3000/SDK/Firmware/BuildOutput/QANI/FreeRTOS/nRF52840DK/Release/qplatform/CMakeFiles/Export/45dfd8a65f6e57dc9beddcd88e943a6b/qplatform_deca_compat-config.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/qplatform_deca_compat/cmake/qplatform_deca_compat-config-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/qplatform_deca_compat/cmake/qplatform_deca_compat-config.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/qplatform_deca_compat/cmake" TYPE FILE FILES "/home/engssg/Android_DWM3000/SDK/Firmware/BuildOutput/QANI/FreeRTOS/nRF52840DK/Release/qplatform/CMakeFiles/Export/45dfd8a65f6e57dc9beddcd88e943a6b/qplatform_deca_compat-config.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/qplatform_deca_compat/cmake" TYPE FILE FILES "/home/engssg/Android_DWM3000/SDK/Firmware/BuildOutput/QANI/FreeRTOS/nRF52840DK/Release/qplatform/CMakeFiles/Export/45dfd8a65f6e57dc9beddcd88e943a6b/qplatform_deca_compat-config-release.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/qplatform" TYPE STATIC_LIBRARY OPTIONAL FILES "/home/engssg/Android_DWM3000/SDK/Firmware/BuildOutput/QANI/FreeRTOS/nRF52840DK/Release/qplatform/libqplatform.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/qplatform/cmake/qplatform-config.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/qplatform/cmake/qplatform-config.cmake"
         "/home/engssg/Android_DWM3000/SDK/Firmware/BuildOutput/QANI/FreeRTOS/nRF52840DK/Release/qplatform/CMakeFiles/Export/ddc7cb44c3e3c3253ccf7687527fe866/qplatform-config.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/qplatform/cmake/qplatform-config-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/qplatform/cmake/qplatform-config.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/qplatform/cmake" TYPE FILE FILES "/home/engssg/Android_DWM3000/SDK/Firmware/BuildOutput/QANI/FreeRTOS/nRF52840DK/Release/qplatform/CMakeFiles/Export/ddc7cb44c3e3c3253ccf7687527fe866/qplatform-config.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/qplatform/cmake" TYPE FILE FILES "/home/engssg/Android_DWM3000/SDK/Firmware/BuildOutput/QANI/FreeRTOS/nRF52840DK/Release/qplatform/CMakeFiles/Export/ddc7cb44c3e3c3253ccf7687527fe866/qplatform-config-release.cmake")
  endif()
endif()

