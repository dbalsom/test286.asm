# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.20)

file(SIZE "${ROM_PATH}" rom_bytes)
if(NOT rom_bytes EQUAL 65536)
    message(FATAL_ERROR "Invalid ROM size: ${rom_bytes}; expected 65536 bytes")
endif()
file(READ "${ROM_PATH}" reset_opcode OFFSET 65520 LIMIT 1 HEX)
if(NOT reset_opcode STREQUAL "ea")
    message(FATAL_ERROR "Invalid reset vector: expected a far JMP at FFF0")
endif()
file(SHA256 "${ROM_PATH}" rom_sha256)
execute_process(COMMAND "${NASM_EXECUTABLE}" -v
    OUTPUT_VARIABLE nasm_version OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY)

function(json_quote output value)
    string(REPLACE "\\" "\\\\" value "${value}")
    string(REPLACE "\"" "\\\"" value "${value}")
    string(REPLACE "\n" "\\n" value "${value}")
    string(REPLACE "\r" "\\r" value "${value}")
    string(REPLACE "\t" "\\t" value "${value}")
    set(${output} "\"${value}\"" PARENT_SCOPE)
endfunction()

file(GLOB_RECURSE sources "${SOURCE_DIR}/src/*.asm" "${SOURCE_DIR}/src/*.inc")
list(APPEND sources "${SOURCE_DIR}/build.cmake" "${CMAKE_CURRENT_LIST_FILE}")
list(SORT sources)
set(source_hashes "{}")
foreach(source IN LISTS sources)
    file(RELATIVE_PATH relative_path "${SOURCE_DIR}" "${source}")
    file(SHA256 "${source}" source_hash)
    string(JSON source_hashes SET "${source_hashes}" "${relative_path}" "\"${source_hash}\"")
endforeach()

set(defines "{}")
foreach(define IN ITEMS POST_PORT OUT_PORT TRACE_ARITH INJECT_FAILURE)
    string(JSON defines SET "${defines}" "${define}" "${${define}}")
endforeach()

set(manifest "{}")
string(JSON manifest SET "${manifest}" format_version 2)
string(JSON manifest SET "${manifest}" rom_sha256 "\"${rom_sha256}\"")
string(JSON manifest SET "${manifest}" rom_bytes "${rom_bytes}")
json_quote(nasm_version_json "${nasm_version}")
string(JSON manifest SET "${manifest}" nasm_version "${nasm_version_json}")
string(JSON manifest SET "${manifest}" source_hashes "${source_hashes}")
string(JSON manifest SET "${manifest}" defines "${defines}")
file(WRITE "${MANIFEST_PATH}" "${manifest}\n")

message(STATUS "ROM: ${ROM_PATH} (${rom_bytes} bytes)")
