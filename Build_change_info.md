Summary
  For UCI nRF52840DK + DWM3000 working with Android, the baseline was:

  - Start from the stock UCI target in SDK/Firmware/Projects/FreeRTOS/UCI/nRF52840DK/
    project_UCI.cmake
  - Build the normal UCI app from SDK/Firmware/Projects/FreeRTOS/UCI/Common/cmakefiles/UCI-
    FreeRTOS.cmake
  - Keep the existing USB transport in SDK/Firmware/Src/Comm/CMakeLists.txt
  - Use the board pin map in SDK/Firmware/Projects/FreeRTOS/UCI/nRF52840DK/ProjectDefinition/
    uwb_stack_llhw.cmake
  - Patch the board L1 config in SDK/Firmware/Src/Boards/Src/nRF52840DK/platform_l1_config.c

  The one firmware change that mattered was in SDK/Firmware/Src/Boards/Src/nRF52840DK/
  platform_l1_config.c:

  - allow PLATFORM_ID_DW3000 and PLATFORM_ID_DW3001C
  - keep the existing QM33110/QM33120 handling
  - keep the DW3001C PVT2 channel-5 antenna-delay workaround

  That is what made the nRF52840DK UCI target accept the DW3000/DW3001C OTP layout instead of
  silently assuming only QM331x.

  The UCI image was rebuilt from:

  - SDK/Firmware/BuildOutput/UCI/FreeRTOS/nRF52840DK/Release

  and the main output is:

  - SDK/Firmware/BuildOutput/UCI/FreeRTOS/nRF52840DK/Release/nRF52840DK-UCI-FreeRTOS.hex
