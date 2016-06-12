cmake_minimum_required(VERSION 3.5)

project(mouros VERSION 0.0.1 LANGUAGES C)


set(ENABLE_DIAGNOSTICS OFF CACHE BOOL "Enable MourOS diagnostics")


if(NOT DEFINED CHIP_FAMILY)
    message(FATAL_ERROR "A STM32 chip family must be defined. -DCHIP_FAMILY=...\nAllowed families are: STM32F0, STM32F4")
endif()

if(NOT (CHIP_FAMILY STREQUAL "STM32F0" OR CHIP_FAMILY STREQUAL "STM32F4"))
    message(FATAL_ERROR "Unknown CHIP_FAMILY: ${CHIP_FAMILY}\nAllowed families are: STM32F0, STM32F4")
endif()



add_library(${PROJECT_NAME} STATIC
    "${CMAKE_CURRENT_LIST_DIR}/src/char_buffer.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/mailbox.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/scheduler.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/sync.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/syscalls.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/tasks.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/pool_alloc.c"
    "${CMAKE_CURRENT_LIST_DIR}/src/diag/diag.h"
)

target_compile_definitions(${PROJECT_NAME} PUBLIC "${CHIP_FAMILY}")

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_INSTALL_PREFIX}/include")

# Link with libopencm3
string(TOLOWER "${CHIP_FAMILY}" CHIP_FAMILY_LCASE)
target_link_libraries(${PROJECT_NAME} INTERFACE "${CMAKE_INSTALL_PREFIX}/lib/libopencm3_${CHIP_FAMILY_LCASE}.a")


# Generate diagnostic functions
add_custom_command(OUTPUT "${CMAKE_CURRENT_LIST_DIR}/src/diag/diag.c" "${CMAKE_CURRENT_LIST_DIR}/src/diag/diag.h"
                   COMMAND "${CMAKE_CURRENT_LIST_DIR}/libsrc/common-stm32/binlog/generate_binlog.py" "diag_config.json" "diag"
                   MAIN_DEPENDENCY "${CMAKE_CURRENT_LIST_DIR}/src/diag/diag_config.json"
                   WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/src/diag")

set_source_files_properties("${CMAKE_CURRENT_LIST_DIR}/src/diag/diag.c" PROPERTIES GENERATED TRUE)


if(ENABLE_DIAGNOSTICS)
    target_compile_definitions(${PROJECT_NAME} PUBLIC "DIAG_ENABLE")
    target_sources(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src/diag/diag.c")
endif()


target_compile_options(${PROJECT_NAME} 
    PUBLIC "-std=gnu11"
           "-fsigned-char" 
           "-ffunction-sections" 
           "-fdata-sections"
    PRIVATE "-Og"
            "-Wunused" 
            "-Wuninitialized" 
            "-Wall" 
            "-Wextra" 
            "-Wmissing-declarations" 
            "-Wconversion" 
            "-Wpointer-arith" 
            "-Wshadow" 
            "-Wlogical-op" 
            "-Waggregate-return" 
            "-Wfloat-equal"
            "$<$<CONFIG:Debug>:-g3>")



install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION "${CMAKE_INSTALL_PREFIX}/lib"
)

install(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/include" DESTINATION "${CMAKE_INSTALL_PREFIX}/include")



