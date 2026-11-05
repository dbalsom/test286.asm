# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.20)

set(SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}")
find_program(NASM_EXECUTABLE NAMES nasm REQUIRED)
option(TRACE_ARITH "Emit the EE arithmetic observation stream" ON)
option(INJECT_FAILURE "Deliberately fail an assertion in POST 02" OFF)
set(POST_PORT "0x190" CACHE STRING "Write-only POST output port")
set(OUT_PORT "0x191" CACHE STRING "Write-only ASCII output port (0 disables text)")
set(OUTPUT_DIR "${SOURCE_DIR}/output" CACHE PATH "Directory for the ROM, listing and manifest")

foreach(port IN ITEMS POST_PORT OUT_PORT)
    if(NOT "${${port}}" MATCHES "^(0x[0-9A-Fa-f]+|[0-9]+)$")
        message(FATAL_ERROR "${port} must be a decimal or 0x-prefixed hexadecimal port")
    endif()
    math(EXPR ${port} "${${port}}")
    if(${port} LESS 0 OR ${port} GREATER 65535)
        message(FATAL_ERROR "${port} must be between 0 and 65535")
    endif()
endforeach()

foreach(switch IN ITEMS TRACE_ARITH INJECT_FAILURE)
    if(${switch})
        set(${switch} 1)
    else()
        set(${switch} 0)
    endif()
endforeach()

get_filename_component(OUTPUT_DIR "${OUTPUT_DIR}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}")
file(MAKE_DIRECTORY "${OUTPUT_DIR}")
set(ROM_PATH "${OUTPUT_DIR}/test286.bin")
set(MANIFEST_PATH "${OUTPUT_DIR}/build.json")
# A failed assembly must not leave a manifest from a previous successful run.
file(REMOVE "${MANIFEST_PATH}")

message(STATUS "Assembling test286 ROM")
execute_process(
    COMMAND "${NASM_EXECUTABLE}"
        -I "${SOURCE_DIR}/src/" -f bin -Wall -Werror
        "-DTRACE_ARITH=${TRACE_ARITH}" "-DINJECT_FAILURE=${INJECT_FAILURE}"
        "-DPOST_PORT=${POST_PORT}" "-DOUT_PORT=${OUT_PORT}"
        -l "${OUTPUT_DIR}/test286.lst" -o "${ROM_PATH}" "${SOURCE_DIR}/src/test286.asm"
    COMMAND_ERROR_IS_FATAL ANY)

include("${SOURCE_DIR}/cmake/WriteManifest.cmake")
