set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

if(NOT DEFINED ARM_GCC_TOOLCHAIN_BIN)
  if(DEFINED ENV{ARM_GCC_TOOLCHAIN_BIN})
    set(ARM_GCC_TOOLCHAIN_BIN "$ENV{ARM_GCC_TOOLCHAIN_BIN}")
  elseif(WIN32)
    file(GLOB _arm_gcc_candidates
      "$ENV{LOCALAPPDATA}/stm32cube/bundles/gnu-tools-for-stm32/*/bin"
      "C:/Program Files*/GNU Arm Embedded Toolchain/*/bin"
      "C:/Program Files*/Arm GNU Toolchain arm-none-eabi/*/bin"
    )
    if(_arm_gcc_candidates)
      list(SORT _arm_gcc_candidates)
      list(REVERSE _arm_gcc_candidates)
      list(GET _arm_gcc_candidates 0 ARM_GCC_TOOLCHAIN_BIN)
    endif()
  endif()
endif()

if(DEFINED ARM_GCC_TOOLCHAIN_BIN)
  file(TO_CMAKE_PATH "${ARM_GCC_TOOLCHAIN_BIN}" ARM_GCC_TOOLCHAIN_BIN)
  set(_arm_gcc_prefix "${ARM_GCC_TOOLCHAIN_BIN}/")
else()
  set(_arm_gcc_prefix "")
endif()

if(WIN32 AND DEFINED ARM_GCC_TOOLCHAIN_BIN)
  set(_arm_gcc_suffix ".exe")
else()
  set(_arm_gcc_suffix "")
endif()

set(CMAKE_C_COMPILER "${_arm_gcc_prefix}arm-none-eabi-gcc${_arm_gcc_suffix}")
set(CMAKE_ASM_COMPILER "${_arm_gcc_prefix}arm-none-eabi-gcc${_arm_gcc_suffix}")
set(CMAKE_OBJCOPY "${_arm_gcc_prefix}arm-none-eabi-objcopy${_arm_gcc_suffix}")
set(CMAKE_SIZE "${_arm_gcc_prefix}arm-none-eabi-size${_arm_gcc_suffix}")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
