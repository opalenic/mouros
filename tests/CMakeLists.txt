cmake_minimum_required(VERSION 3.5)

project(mouros-tests VERSION 0.0.1 LANGUAGES C)

enable_testing()


# Find cmocka
find_library(CMOCKA_LIB cmocka)
link_libraries("${CMOCKA_LIB}")

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
    "$<$<CONFIG:Debug>:-g>"
)



add_executable(test_pool_alloc
    "${CMAKE_CURRENT_LIST_DIR}/../src/pool_alloc.c"
    "${CMAKE_CURRENT_LIST_DIR}/test_pool_alloc.c"
)

add_test(NAME test_pool_alloc COMMAND test_pool_alloc)


