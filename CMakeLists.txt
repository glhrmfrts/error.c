cmake_minimum_required( VERSION 3.15 )

project( error_test )

set( TARGET_NAME error_test )

add_executable( ${TARGET_NAME} error_test.c )

target_compile_options( ${TARGET_NAME} PRIVATE -std=c11 -Werror -Wall -Wextra -pedantic -g )