
include (CMakeForceCompiler)
CMAKE_FORCE_C_COMPILER(arm-none-eabi-gcc ArmEmbeddedGCC)

set(TARGET_TRIPLET "arm-none-eabi")

set(CMAKE_SYSTEM_PROCESSOR "arm")
set(CMAKE_SYSTEM_NAME "Generic")
set(CMAKE_SYSTEM_VERSION "Generic")

set(CMAKE_SYSROOT "/usr/${TARGET_TRIPLET}")


if(CHIP_FAMILY STREQUAL "STM32F0")
    set(CMAKE_C_FLAGS "-mcpu=cortex-m0 -msoft-float -mthumb" CACHE STRING "cflags")
elseif(CHIP_FAMILY STREQUAL "STM32F4")
    set(CMAKE_C_FLAGS "-mcpu=cortex-m4f -mfloat-abi=hard -mfpu=fpv4-sp-d16 -mthumb" CACHE STRING "cflags")
else()
    message(FATAL_ERROR "Unknown CHIP_FAMILY: ${CHIP_FAMILY}\nAllowed families are: STM32F0, STM32F4")
endif()
