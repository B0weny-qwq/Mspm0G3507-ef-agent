cmake_minimum_required(VERSION 3.20)
project(mspm0g3507 C ASM)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT DEFINED MSPM0_SDK_PATH)
  if(DEFINED ENV{MSPM0_SDK_PATH})
    set(MSPM0_SDK_PATH "$ENV{MSPM0_SDK_PATH}")
  else()
    set(MSPM0_SDK_PATH "$ENV{HOME}/SDK/TI/mspm0-sdk")
  endif()
endif()

set(SDK "${MSPM0_SDK_PATH}")
set(DRIVERLIB "${SDK}/source/ti/driverlib/lib/gcc/m0p/mspm0g1x0x_g3x0x/driverlib.a")
set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/linker/mspm0g3507.lds")

set(MSPM0_COMPILE_OPTIONS
  -mcpu=cortex-m0plus
  -march=armv6-m
  -mthumb
  -mfloat-abi=soft
  -std=c99
  -ffunction-sections
  -fdata-sections
  -g
  -gstrict-dwarf
  -Wall
  -Wextra
  -O2
)

add_library(ef_platform
  Platform/Src/ef_platform.c
  Platform/Src/ti_msp_dl_config.c
)

target_include_directories(ef_platform
  PUBLIC Platform/Inc
  PRIVATE
    ${SDK}/source
    ${SDK}/source/third_party/CMSIS/Core/Include
)

target_compile_definitions(ef_platform PUBLIC __MSPM0G3507__)
target_compile_options(ef_platform PRIVATE ${MSPM0_COMPILE_OPTIONS})

add_library(ef_drivers
  Drivers/Src/ef_gpio_mspm0.c
  Drivers/Src/ef_spi_mspm0.c
  Drivers/Src/ef_uart_mspm0.c
)

target_include_directories(ef_drivers PUBLIC Drivers/Inc)
target_include_directories(ef_drivers PRIVATE
  ${SDK}/source
  ${SDK}/source/third_party/CMSIS/Core/Include
)
target_link_libraries(ef_drivers PRIVATE ef_platform)
target_compile_options(ef_drivers PRIVATE ${MSPM0_COMPILE_OPTIONS})

add_library(ef_chipdrivers
  ChipDrivers/Src/st7789.c
  ChipDrivers/Src/w25q128.c
)

target_include_directories(ef_chipdrivers PUBLIC ChipDrivers/Inc)
target_compile_options(ef_chipdrivers PRIVATE ${MSPM0_COMPILE_OPTIONS})

add_library(ef_boarddevices
  BoardDevices/Src/board_console.c
  BoardDevices/Src/board_flash.c
  BoardDevices/Src/board_lcd.c
  BoardDevices/Src/board_led.c
)

target_include_directories(ef_boarddevices PUBLIC BoardDevices/Inc)
target_link_libraries(ef_boarddevices PRIVATE ef_drivers ef_chipdrivers ef_platform)
target_compile_options(ef_boarddevices PRIVATE ${MSPM0_COMPILE_OPTIONS})

add_library(ef_components
  Components/Src/ef_component_placeholder.c
)

target_include_directories(ef_components PUBLIC Components/Inc)
target_compile_options(ef_components PRIVATE ${MSPM0_COMPILE_OPTIONS})

add_library(ef_services
  Services/Src/ef_event.c
  Services/Src/ef_log.c
  Services/Src/ef_scheduler.c
)

target_include_directories(ef_services PUBLIC Services/Inc)
target_link_libraries(ef_services PRIVATE ef_boarddevices ef_platform)
target_compile_options(ef_services PRIVATE ${MSPM0_COMPILE_OPTIONS})

add_library(ef_app
  App/Src/app.c
)

target_include_directories(ef_app PUBLIC App/Inc)
target_link_libraries(ef_app PRIVATE ef_boarddevices ef_components ef_services)
target_compile_options(ef_app PRIVATE ${MSPM0_COMPILE_OPTIONS})

add_executable(app.elf
  App/Src/main.c
  Startup/startup_mspm0g350x_gcc.c
)

target_compile_options(app.elf PRIVATE ${MSPM0_COMPILE_OPTIONS})
target_link_libraries(app.elf PRIVATE
  ef_app
  ef_platform
  ef_services
  ${DRIVERLIB}
  gcc
  c
  m
)

target_link_options(app.elf PRIVATE
  -mcpu=cortex-m0plus
  -march=armv6-m
  -mthumb
  -mfloat-abi=soft
  -nostartfiles
  -static
  -Wl,--gc-sections
  -Wl,-T,${LINKER_SCRIPT}
  -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/app.map
  --specs=nano.specs
  --specs=nosys.specs
)

add_custom_command(TARGET app.elf POST_BUILD
  COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:app.elf> ${CMAKE_CURRENT_BINARY_DIR}/app.hex
  COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:app.elf> ${CMAKE_CURRENT_BINARY_DIR}/app.bin
  COMMAND ${CMAKE_SIZE} $<TARGET_FILE:app.elf>
)
