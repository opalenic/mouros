cmake_minimum_required(VERSION 3.5)

project(mouros-tests VERSION 0.0.1 LANGUAGES C)



include(ExternalProject)

get_filename_component(TOOLCHAIN_FILE_ABSPATH "${CMAKE_TOOLCHAIN_FILE}" ABSOLUTE)

# Build cmocka
ExternalProject_Add(cmocka
    URL https://cmocka.org/files/1.0/cmocka-1.0.1.tar.xz
    URL_MD5 ed861e501a21a92b2af63e466df2015e
    CMAKE_ARGS "$<$<BOOL:${CMAKE_TOOLCHAIN_FILE}>:-DCMAKE_TOOLCHAIN_FILE:string=${TOOLCHAIN_FILE_ABSPATH}>"
    CMAKE_ARGS "$<$<BOOL:${CMAKE_INSTALL_PREFIX}>:-DCMAKE_INSTALL_PREFIX:string=${CMAKE_INSTALL_PREFIX}>"
)


enable_testing()

# Link cmocka
link_libraries("${CMAKE_BINARY_DIR}/cmocka-prefix/src/cmocka-build/src/libcmocka.so")

# Enable coverage
link_libraries("--coverage")

# Add include stub overrides & MourOS includes
include_directories(
    "${CMAKE_CURRENT_LIST_DIR}/stubs/include"
    "${CMAKE_CURRENT_LIST_DIR}/../include"
)

# Add public include dirs
include_directories(SYSTEM
    "${CMAKE_INSTALL_PREFIX}/include"
)

# Add a few warnings
add_compile_options(
    "-std=gnu11"
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
    "$<$<CONFIG:Debug>:-g3>"
)



# Pool allocator tests
add_executable(test_pool_alloc
    "${CMAKE_CURRENT_LIST_DIR}/../include/mouros/pool_alloc.h"
    "${CMAKE_CURRENT_LIST_DIR}/../src/pool_alloc.c"
    "${CMAKE_CURRENT_LIST_DIR}/test_pool_alloc.c"
)

set_source_files_properties("${CMAKE_CURRENT_LIST_DIR}/../src/pool_alloc.c" PROPERTIES COMPILE_FLAGS "--coverage")

add_test(NAME pool_alloc COMMAND test_pool_alloc)
set_tests_properties(pool_alloc PROPERTIES DEPENDS test_pool_alloc)



# Covearge
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/coverage")

add_custom_target(coverage
    COMMAND gcovr -r "${CMAKE_CURRENT_LIST_DIR}/../src" --html --html-details -b -o coverage/${PROJECT_NAME}-coverage.html .
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
)

