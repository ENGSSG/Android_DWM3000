# Install script for directory: /home/engssg/Android_DWM3000/SDK/Firmware/Projects/FreeRTOS/UCI/nRF52840DK

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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/arm-cortex-m4-hard_floating/uwbdriver" TYPE STATIC_LIBRARY OPTIONAL FILES "/home/engssg/Android_DWM3000/SDK/Firmware/BuildOutput/UCI/FreeRTOS/nRF52840DK/Release/libuwbdriver.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/arm-cortex-m4-hard_floating/uwbdriver/cmake/uwbdriver-config.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/arm-cortex-m4-hard_floating/uwbdriver/cmake/uwbdriver-config.cmake"
         "/home/engssg/Android_DWM3000/SDK/Firmware/BuildOutput/UCI/FreeRTOS/nRF52840DK/Release/CMakeFiles/Export/bc21a1fca165660f0ece2b504d6b24bf/uwbdriver-config.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/arm-cortex-m4-hard_floating/uwbdriver/cmake/uwbdriver-config-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/arm-cortex-m4-hard_floating/uwbdriver/cmake/uwbdriver-config.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/arm-cortex-m4-hard_floating/uwbdriver/cmake" TYPE FILE FILES "/home/engssg/Android_DWM3000/SDK/Firmware/BuildOutput/UCI/FreeRTOS/nRF52840DK/Release/CMakeFiles/Export/bc21a1fca165660f0ece2b504d6b24bf/uwbdriver-config.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/arm-cortex-m4-hard_floating/uwbdriver/cmake" TYPE FILE FILES "/home/engssg/Android_DWM3000/SDK/Firmware/BuildOutput/UCI/FreeRTOS/nRF52840DK/Release/CMakeFiles/Export/bc21a1fca165660f0ece2b504d6b24bf/uwbdriver-config-release.cmake")
  endif()
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/engssg/Android_DWM3000/SDK/Firmware/BuildOutput/UCI/FreeRTOS/nRF52840DK/Release/ProjectDefinition/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/engssg/Android_DWM3000/SDK/Firmware/BuildOutput/UCI/FreeRTOS/nRF52840DK/Release/Nordic/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/engssg/Android_DWM3000/SDK/Firmware/BuildOutput/UCI/FreeRTOS/nRF52840DK/Release/FreeRTOS/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/engssg/Android_DWM3000/SDK/Firmware/BuildOutput/UCI/FreeRTOS/nRF52840DK/Release/EventManager/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/engssg/Android_DWM3000/SDK/Firmware/BuildOutput/UCI/FreeRTOS/nRF52840DK/Release/OSAL/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/engssg/Android_DWM3000/SDK/Firmware/BuildOutput/UCI/FreeRTOS/nRF52840DK/Release/nRF52840DK/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/engssg/Android_DWM3000/SDK/Firmware/BuildOutput/UCI/FreeRTOS/nRF52840DK/Release/HAL/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/engssg/Android_DWM3000/SDK/Firmware/BuildOutput/UCI/FreeRTOS/nRF52840DK/Release/qhal/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/engssg/Android_DWM3000/SDK/Firmware/BuildOutput/UCI/FreeRTOS/nRF52840DK/Release/qplatform/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/engssg/Android_DWM3000/SDK/Firmware/BuildOutput/UCI/FreeRTOS/nRF52840DK/Release/logger/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/engssg/Android_DWM3000/SDK/Firmware/BuildOutput/UCI/FreeRTOS/nRF52840DK/Release/AppConfig/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/engssg/Android_DWM3000/SDK/Firmware/BuildOutput/UCI/FreeRTOS/nRF52840DK/Release/AppCommon/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/engssg/Android_DWM3000/SDK/Firmware/BuildOutput/UCI/FreeRTOS/nRF52840DK/Release/UWB/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/engssg/Android_DWM3000/SDK/Firmware/BuildOutput/UCI/FreeRTOS/nRF52840DK/Release/AppFira/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/engssg/Android_DWM3000/SDK/Firmware/BuildOutput/UCI/FreeRTOS/nRF52840DK/Release/reporter/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/engssg/Android_DWM3000/SDK/Firmware/BuildOutput/UCI/FreeRTOS/nRF52840DK/Release/Helpers/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/engssg/Android_DWM3000/SDK/Firmware/BuildOutput/UCI/FreeRTOS/nRF52840DK/Release/BLE/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/engssg/Android_DWM3000/SDK/Firmware/BuildOutput/UCI/FreeRTOS/nRF52840DK/Release/Comm/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/engssg/Android_DWM3000/SDK/Firmware/BuildOutput/UCI/FreeRTOS/nRF52840DK/Release/uci/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/engssg/Android_DWM3000/SDK/Firmware/BuildOutput/UCI/FreeRTOS/nRF52840DK/Release/SoftDeviceNrfx/cmake_install.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/home/engssg/Android_DWM3000/SDK/Firmware/BuildOutput/UCI/FreeRTOS/nRF52840DK/Release/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
