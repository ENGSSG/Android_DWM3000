# Install script for directory: /home/engssg/Android_DWM3000/SDK/Firmware/Libs/uwb-stack/libs/qosal

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
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
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
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/arm-cortex-m4-hard_floating/qosal_itf/cmake/qosal_itf-config.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/arm-cortex-m4-hard_floating/qosal_itf/cmake/qosal_itf-config.cmake"
         "/home/engssg/Android_DWM3000/SDK/Firmware/BuildOutput/UCI/FreeRTOS/nRF52840DK/Debug/OSAL/qosal/CMakeFiles/Export/fda953f2e90e39966b1cc079bbd67c76/qosal_itf-config.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/arm-cortex-m4-hard_floating/qosal_itf/cmake/qosal_itf-config-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/arm-cortex-m4-hard_floating/qosal_itf/cmake/qosal_itf-config.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/arm-cortex-m4-hard_floating/qosal_itf/cmake" TYPE FILE FILES "/home/engssg/Android_DWM3000/SDK/Firmware/BuildOutput/UCI/FreeRTOS/nRF52840DK/Debug/OSAL/qosal/CMakeFiles/Export/fda953f2e90e39966b1cc079bbd67c76/qosal_itf-config.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/arm-cortex-m4-hard_floating/qosal" TYPE STATIC_LIBRARY FILES "/home/engssg/Android_DWM3000/SDK/Firmware/BuildOutput/UCI/FreeRTOS/nRF52840DK/Debug/OSAL/qosal/libqosal.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/arm-cortex-m4-hard_floating/qosal/cmake/qosal-config.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/arm-cortex-m4-hard_floating/qosal/cmake/qosal-config.cmake"
         "/home/engssg/Android_DWM3000/SDK/Firmware/BuildOutput/UCI/FreeRTOS/nRF52840DK/Debug/OSAL/qosal/CMakeFiles/Export/6b12b39fccbba8f01ea3af382f866741/qosal-config.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/arm-cortex-m4-hard_floating/qosal/cmake/qosal-config-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/arm-cortex-m4-hard_floating/qosal/cmake/qosal-config.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/arm-cortex-m4-hard_floating/qosal/cmake" TYPE FILE FILES "/home/engssg/Android_DWM3000/SDK/Firmware/BuildOutput/UCI/FreeRTOS/nRF52840DK/Debug/OSAL/qosal/CMakeFiles/Export/6b12b39fccbba8f01ea3af382f866741/qosal-config.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/arm-cortex-m4-hard_floating/qosal/cmake" TYPE FILE FILES "/home/engssg/Android_DWM3000/SDK/Firmware/BuildOutput/UCI/FreeRTOS/nRF52840DK/Debug/OSAL/qosal/CMakeFiles/Export/6b12b39fccbba8f01ea3af382f866741/qosal-config-debug.cmake")
  endif()
endif()

