cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

if(POLICY CMP0092)
    cmake_policy(SET CMP0092 NEW) # Don't add -W3 warning level by default.
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Use statically-linked runtime library in Release build.
# https://cmake.org/cmake/help/git-stage/variable/CMAKE_MSVC_RUNTIME_LIBRARY.html
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    cmake_policy(SET CMP0091 NEW)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
else()
    cmake_policy(SET CMP0091 NEW)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
    add_definitions(-DDEBUG)
endif()


project(plugify-module-v8
        VERSION 1.0.0.0
        DESCRIPTION "JavaScript (V8) Language Module"
        HOMEPAGE_URL "https://github.com/untrustedmodders/plugify-module-v8"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

#
# Plugify
#
set(PLUGIFY_BUILD_SHARED_LIB ON CACHE INTERNAL "")
set(PLUGIFY_BUILD_JIT ON CACHE INTERNAL "")
set(PLUGIFY_BUILD_ASSEMBLY OFF CACHE INTERNAL "")
set(PLUGIFY_BUILD_TESTS OFF CACHE INTERNAL "")
set(PLUGIFY_INTERFACE ON CACHE INTERNAL "")
set(PLUGIFY_DOWNLOADER OFF CACHE INTERNAL "")
add_subdirectory(external/plugify)

#
# V8 Language Module for Plugify
#
file(GLOB_RECURSE V8LM_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.cpp")
set(V8LM_PCH_FILE "src/pch.hpp")

add_library(${PROJECT_NAME} SHARED ${V8LM_SOURCES})
target_precompile_headers(${PROJECT_NAME} PRIVATE ${V8LM_PCH_FILE})

#
# V8 Library
#
set(V8LM_EXTERNAL ON)
if(V8LM_EXTERNAL)
    add_library(v8_monolith SHARED IMPORTED)
    set(V8_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/v8/include/")
    if(WIN32)
        set(V8LM_V8_RELEASE_LIB "${CMAKE_CURRENT_SOURCE_DIR}/v8/libs/windows/v8.dll")
        set(V8LM_V8_RELEASE_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/v8/libs/windows/v8.lib")

        set_property(TARGET v8_monolith PROPERTY IMPORTED_LOCATION_RELEASE  ${V8LM_V8_RELEASE_LIB})
        set_property(TARGET v8_monolith PROPERTY IMPORTED_IMPLIB_RELEASE ${V8LM_V8_RELEASE_IMPLIB})
        set_property(TARGET v8_monolith PROPERTY IMPORTED_LOCATION_RELWITHDEBINFO ${V8LM_V8_RELEASE_LIB})
        set_property(TARGET v8_monolith PROPERTY IMPORTED_IMPLIB_RELWITHDEBINFO ${V8LM_V8_RELEASE_IMPLIB})
        set_property(TARGET v8_monolith PROPERTY IMPORTED_LOCATION_MINSIZEREL ${V8LM_V8_RELEASE_LIB})
        set_property(TARGET v8_monolith PROPERTY IMPORTED_IMPLIB_MINSIZEREL ${V8LM_V8_RELEASE_IMPLIB})

        set(V8LM_LINK_LIBRARIES winmm dbghelp)
    elseif(LINUX)
        set(V8LM_V8_RELEASE_LIB "${CMAKE_CURRENT_SOURCE_DIR}/v8/libs/linux/libv8.so")

        set_property(TARGET v8_monolith PROPERTY IMPORTED_LOCATION_RELEASE ${V8LM_V8_RELEASE_LIB})
        set_property(TARGET v8_monolith PROPERTY IMPORTED_LOCATION_RELWITHDEBINFO ${V8LM_V8_RELEASE_LIB})
        set_property(TARGET v8_monolith PROPERTY IMPORTED_LOCATION_MINSIZEREL ${V8LM_V8_RELEASE_LIB})

        set(V8LM_LINK_LIBRARIES pthread)
    endif()
else()
    if(WIN32)
        set(V8_VERSION "11.9.169") # 13.1.2
        set(V8_CONFIG_STR "x64.release")
        set(V8_IS_DEBUG OFF)
        set(V8_TARGET_NAME "v8_monolith")
        set(V8_IS_COMPONENT_BUILD OFF)
        set(V8_IS_CLANG OFF)
        set(V8_TARGET_CPU "x64")
        set(V8_USE_CUSTOM_LIBCXX OFF)
        set(V8_USE_LLD OFF)
        set(V8_V8_MONOLITHIC ON)
        set(V8_V8_USE_EXTERNAL_STARTUP_DATA OFF)
        set(V8LM_LINK_LIBRARIES winmm dbghelp)
    else()
        set(V8_VERSION "11.9.169") # 13.1.2
        set(V8_CONFIG_STR "x64.release")
        set(V8_IS_DEBUG OFF)
        set(V8_TARGET_NAME "v8_monolith")
        set(V8_IS_COMPONENT_BUILD OFF)
        set(V8_IS_CLANG OFF)
        set(V8_TARGET_CPU "x64")
        set(V8_USE_CUSTOM_LIBCXX OFF)
        set(V8_USE_LLD OFF)
        set(V8_V8_MONOLITHIC ON)
        set(V8_V8_USE_EXTERNAL_STARTUP_DATA OFF)
        set(V8LM_LINK_LIBRARIES pthread)
    endif()
    set(V8_ROOT ${CMAKE_BINARY_DIR})
    add_subdirectory(external/v8)
endif()

if(NOT COMPILER_SUPPORTS_FORMAT)
    set(V8LM_LINK_LIBRARIES ${V8LM_LINK_LIBRARIES} fmt::fmt-header-only)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${V8_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${V8_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE plugify::plugify plugify::plugify-jit asmjit::asmjit v8_monolith ${V8LM_LINK_LIBRARIES})
target_compile_definitions(${PROJECT_NAME} PRIVATE ${V8_COMPILE_DEFINITIONS})
#target_compile_definitions(${PROJECT_NAME} PRIVATE -D_ITERATOR_DEBUG_LEVEL=0)

if(MSVC)
    #target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX /Zc:__cplusplus)
    target_compile_options(${PROJECT_NAME} PRIVATE /Zc:__cplusplus)
else()
    #target_compile_options(${PROJECT_NAME} PRIVATE -Wextra -Wshadow -Wconversion -Wpedantic -Werror)
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE
       V8LM_NAME="${PROJECT_NAME}"
       V8LM_VERSION="${CMAKE_PROJECT_VERSION}"
       V8LM_OS="${CMAKE_SYSTEM_NAME}"
       V8LM_PLATFORM="${PLUGIFY_PLATFORM}"
       V8LM_PLATFORM_WINDOWS=$<BOOL:${WIN32}>
       V8LM_PLATFORM_APPLE=$<BOOL:${APPLE}>
       V8LM_PLATFORM_LINUX=$<BOOL:${LINUX}>
       V8LM_PLATFORM_ANDROID=$<BOOL:${ANDROID}>
       V8LM_PLATFORM_BSD=$<BOOL:${BSD}>
       V8LM_PLATFORM_ORBIS=$<BOOL:${ORBIS}>
       V8LM_PLATFORM_PROSPERO=$<BOOL:${PROSPERO}>
       V8LM_PLATFORM_SWITCH=$<BOOL:${SWITCH}>
       V8LM_ARCH_BITS=$<IF:$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>,64,32>
       V8LM_ARCH_ARM=$<BOOL:${PLUGIFY_USE_ARM}>
       V8LM_IS_BIG_ENDIAN=$<STREQUAL:${CMAKE_CXX_BYTE_ORDER},BIG_ENDIAN>
       V8LM_IS_DEBUG=$<STREQUAL:${CMAKE_BUILD_TYPE},Debug>
       V8LM_IS_RELEASE=$<STREQUAL:${CMAKE_BUILD_TYPE},Release>
       V8LM_EXTERNAL=$<BOOL:${V8LM_EXTERNAL}>
)

# Generate the embedded default (welcome) source.
add_custom_command(
        OUTPUT generated_default.cc
        COMMAND embed ${CMAKE_CURRENT_BINARY_DIR}/generated_default.cc kEmbeddedDefaultSource ./src/default.js
        DEPENDS embed default.js
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..
)

include(GenerateExportHeader)
generate_export_header(${PROJECT_NAME} EXPORT_MACRO_NAME V8LM_EXPORT EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/exports/module_export.h)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR}/exports)

if(APPLE)
    target_link_options(${PROJECT_NAME} PRIVATE "-Wl,-exported_symbols_list,${CMAKE_CURRENT_SOURCE_DIR}/sym/exported_symbols.lds")
elseif(UNIX)
    target_link_options(${PROJECT_NAME} PRIVATE "-Wl,--version-script,${CMAKE_CURRENT_SOURCE_DIR}/sym/version_script.lds")
endif()

file(READ "${CMAKE_SOURCE_DIR}/version.txt" VERSION_FILE_CONTENTS)
set(V8LM_VERSION "${CMAKE_PROJECT_VERSION}" CACHE STRING "Set version name")
set(V8LM_PACKAGE "${PROJECT_NAME}" CACHE STRING "Set package name")
string(REPLACE "v" "" V8LM_PACKAGE "${V8LM_PACKAGE}")

configure_file(
    ${CMAKE_SOURCE_DIR}/${PROJECT_NAME}.pmodule.in
    ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pmodule
)

set(V8LM_GITHUB_ACTIONS "0" CACHE STRING "Build with github actions")

if(V8LM_GITHUB_ACTIONS)
    execute_process(COMMAND cmake -E create_symlink
            "${CMAKE_SOURCE_DIR}/lib"
            "${CMAKE_BINARY_DIR}/lib"
    )
endif()

