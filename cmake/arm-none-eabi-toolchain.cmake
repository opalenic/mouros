
include (CMakeForceCompiler)
CMAKE_FORCE_C_COMPILER(arm-none-eabi-gcc ArmEmbeddedGCC)

set(TARGET_TRIPLET "arm-none-eabi")

set(CMAKE_SYSTEM_PROCESSOR "arm")
set(CMAKE_SYSTEM_NAME "Generic")
set(CMAKE_SYSTEM_VERSION "Generic")

set(CMAKE_SYSROOT "/usr/${TARGET_TRIPLET}")
