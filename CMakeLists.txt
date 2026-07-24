cmake_minimum_required(VERSION 3.10)
project(chttp C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(LIB_SOURCES
    src/handler.c
    src/parser.c
    src/response.c
    src/server_win.c
    src/helper.c
)

add_library(chttp STATIC ${LIB_SOURCES})
target_include_directories(chttp PRIVATE src/inc)
target_compile_options(chttp PRIVATE -Wall -Wextra)

add_executable(chttp_server main.c)
target_include_directories(chttp_server PRIVATE src/inc)
target_compile_options(chttp_server PRIVATE -Wall -Wextra)


if(WIN32)
    target_link_libraries(chttp_server PRIVATE chttp ws2_32)
else()
    message(FATAL_ERROR "This project supports only Windows")
endif()
