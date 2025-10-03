cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

if(POLICY CMP0092)
    cmake_policy(SET CMP0092 NEW) # Don't add -W3 warning level by default.
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


file(READ "${CMAKE_CURRENT_SOURCE_DIR}/version.txt" VERSION_FILE_CONTENTS)
string(STRIP "${VERSION_FILE_CONTENTS}" VERSION_FILE_CONTENTS)
set(V8LM_VERSION "${VERSION_FILE_CONTENTS}" CACHE STRING "Set version name")
set(V8LM_PACKAGE "v8_module" CACHE STRING "Set package name")
set(V8LM_STATIC OFF CACHE BOOL "Build v8 as static?")
string(REPLACE "v" "" V8LM_VERSION "${V8LM_VERSION}")
string(REGEX REPLACE "[.+-]" ";" V8LM_VERSION_LIST ${V8LM_VERSION})
list(GET V8LM_VERSION_LIST 0 V8LM_VERSION_MAJOR)
list(GET V8LM_VERSION_LIST 1 V8LM_VERSION_MINOR)
list(GET V8LM_VERSION_LIST 2 V8LM_VERSION_PATCH)

project(plugify-module-v8
        VERSION "${V8LM_VERSION_MAJOR}.${V8LM_VERSION_MINOR}.${V8LM_VERSION_PATCH}"
        DESCRIPTION "JavaScript (V8) Language Module"
        HOMEPAGE_URL "https://github.com/untrustedmodders/plugify-module-v8"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
#set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(NOT CMAKE_BUILD_TYPE MATCHES "Debug|Devel|MinSizeRel|RelWithDebInfo|Release")
    message(STATUS "CMAKE_BUILD_TYPE not set, defaulting to Release.")
    set(CMAKE_BUILD_TYPE Release)
endif()

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

set(BUILD_SHARED_LIBS ON CACHE INTERNAL "")

#
# Plugify
#
set(PLUGIFY_BUILD_SHARED_LIB ON CACHE INTERNAL "")
set(PLUGIFY_BUILD_TESTS OFF CACHE INTERNAL "")
if(LINUX)
	set(PLUGIFY_USE_STATIC_STDLIB ON CACHE INTERNAL "")
	set(PLUGIFY_USE_ABI0 OFF CACHE INTERNAL "")
endif()
set(PLUGIFY_USE_SANITIZER ON CACHE INTERNAL "")
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
if(NOT V8LM_STATIC)
    set(V8_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/v8/include/")
    if(WIN32)
        set(V8LM_LINK_LIBRARIES winmm dbghelp)
        set(V8_COMPILE_DEFINITIONS _CRT_SECURE_NO_WARNINGS=1)
    elseif(LINUX)
        set(V8LM_LINK_LIBRARIES pthread)
    endif()
    add_library(v8_monolith SHARED IMPORTED)
    if(WIN32)
        set(V8LM_V8_RELEASE_LIB "${CMAKE_CURRENT_SOURCE_DIR}/v8/libs/windows/v8.dll")
        set(V8LM_V8_RELEASE_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/v8/libs/windows/v8.lib")

        set_property(TARGET v8_monolith PROPERTY IMPORTED_LOCATION ${V8LM_V8_RELEASE_LIB})
        set_property(TARGET v8_monolith PROPERTY IMPORTED_IMPLIB ${V8LM_V8_RELEASE_IMPLIB})
        set_property(TARGET v8_monolith PROPERTY IMPORTED_LOCATION_RELEASE  ${V8LM_V8_RELEASE_LIB})
        set_property(TARGET v8_monolith PROPERTY IMPORTED_IMPLIB_RELEASE ${V8LM_V8_RELEASE_IMPLIB})
        set_property(TARGET v8_monolith PROPERTY IMPORTED_LOCATION_RELWITHDEBINFO ${V8LM_V8_RELEASE_LIB})
        set_property(TARGET v8_monolith PROPERTY IMPORTED_IMPLIB_RELWITHDEBINFO ${V8LM_V8_RELEASE_IMPLIB})
        set_property(TARGET v8_monolith PROPERTY IMPORTED_LOCATION_MINSIZEREL ${V8LM_V8_RELEASE_LIB})
        set_property(TARGET v8_monolith PROPERTY IMPORTED_IMPLIB_MINSIZEREL ${V8LM_V8_RELEASE_IMPLIB})
    elseif(LINUX)
        set(V8LM_V8_RELEASE_LIB "${CMAKE_CURRENT_SOURCE_DIR}/v8/libs/linux/libv8.so")

        set_property(TARGET v8_monolith PROPERTY IMPORTED_LOCATION ${V8LM_V8_RELEASE_LIB})
        set_property(TARGET v8_monolith PROPERTY IMPORTED_LOCATION_RELEASE ${V8LM_V8_RELEASE_LIB})
        set_property(TARGET v8_monolith PROPERTY IMPORTED_LOCATION_RELWITHDEBINFO ${V8LM_V8_RELEASE_LIB})
        set_property(TARGET v8_monolith PROPERTY IMPORTED_LOCATION_MINSIZEREL ${V8LM_V8_RELEASE_LIB})
    endif()
    add_library(v8_libplatform SHARED IMPORTED)
    if(WIN32)
        set(V8LM_V8_PLAT_RELEASE_LIB "${CMAKE_CURRENT_SOURCE_DIR}/v8/libs/windows/v8_libplatform.dll")
        set(V8LM_V8_PLAT_RELEASE_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/v8/libs/windows/v8_libplatform.lib")

        set_property(TARGET v8_libplatform PROPERTY IMPORTED_LOCATION ${V8LM_V8_PLAT_RELEASE_LIB})
        set_property(TARGET v8_libplatform PROPERTY IMPORTED_IMPLIB ${V8LM_V8_PLAT_RELEASE_IMPLIB})
        set_property(TARGET v8_libplatform PROPERTY IMPORTED_LOCATION_RELEASE  ${V8LM_V8_PLAT_RELEASE_LIB})
        set_property(TARGET v8_libplatform PROPERTY IMPORTED_IMPLIB_RELEASE ${V8LM_V8_PLAT_RELEASE_IMPLIB})
        set_property(TARGET v8_libplatform PROPERTY IMPORTED_LOCATION_RELWITHDEBINFO ${V8LM_V8_PLAT_RELEASE_LIB})
        set_property(TARGET v8_libplatform PROPERTY IMPORTED_IMPLIB_RELWITHDEBINFO ${V8LM_V8_PLAT_RELEASE_IMPLIB})
        set_property(TARGET v8_libplatform PROPERTY IMPORTED_LOCATION_MINSIZEREL ${V8LM_V8_PLAT_RELEASE_LIB})
        set_property(TARGET v8_libplatform PROPERTY IMPORTED_IMPLIB_MINSIZEREL ${V8LM_V8_PLAT_RELEASE_IMPLIB})
    elseif(LINUX)
        set(V8LM_V8_PLAT_RELEASE_LIB "${CMAKE_CURRENT_SOURCE_DIR}/v8/libs/linux/libv8_libplatform.so")

        set_property(TARGET v8_libplatform PROPERTY IMPORTED_LOCATION ${V8LM_V8_PLAT_RELEASE_LIB})
        set_property(TARGET v8_libplatform PROPERTY IMPORTED_LOCATION_RELEASE ${V8LM_V8_PLAT_RELEASE_LIB})
        set_property(TARGET v8_libplatform PROPERTY IMPORTED_LOCATION_RELWITHDEBINFO ${V8LM_V8_PLAT_RELEASE_LIB})
        set_property(TARGET v8_libplatform PROPERTY IMPORTED_LOCATION_MINSIZEREL ${V8LM_V8_PLAT_RELEASE_LIB})
    endif()
    set(V8LM_LINK_LIBRARIES ${V8LM_LINK_LIBRARIES} v8_libplatform)
    target_include_directories(v8_monolith INTERFACE ${V8_INCLUDE_DIRS})
    target_link_libraries(v8_monolith INTERFACE ${V8LM_LINK_LIBRARIES})
    if(MSVC)
        target_compile_options(v8_monolith INTERFACE /wd4100 /wd4244)
    else()
        target_compile_options(v8_monolith INTERFACE -Wno-unused-parameter -Wno-conversion -Wno-template-id-cdtor -Wno-shadow)
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

target_link_directories(${PROJECT_NAME} PRIVATE ${V8_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE plugify::plugify v8_monolith ${V8LM_LINK_LIBRARIES})
target_compile_definitions(${PROJECT_NAME} PRIVATE ${V8_COMPILE_DEFINITIONS})
#target_compile_definitions(${PROJECT_NAME} PRIVATE -D_ITERATOR_DEBUG_LEVEL=0)
#target_compile_definitions(${PROJECT_NAME} PRIVATE -D_GLIBCXX_USE_CXX11_ABI=0)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE winhttp.lib)
else()
    find_package(CURL REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE CURL::libcurl)
    message(STATUS "Found CURL version: ${CURL_VERSION_STRING}")
    message(STATUS "Using CURL include dir(s): ${CURL_INCLUDE_DIRS}")
    message(STATUS "Using CURL lib(s): ${CURL_LIBRARIES}")
endif()

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX /Zc:__cplusplus /Zc:preprocessor $<$<CONFIG:Release>:/Zi>)
    target_link_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:/DEBUG /OPT:REF /OPT:ICF>)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wextra -Wconversion -Wpedantic -Werror -Wshadow $<$<CONFIG:Release>:-g1>)
endif()

# Generate the embedded default (welcome) source.
add_custom_command(
        OUTPUT generated_default.cc
        COMMAND embed ${CMAKE_CURRENT_BINARY_DIR}/generated_default.cc kEmbeddedDefaultSource ./src/default.js
        DEPENDS embed default.js
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..
)

if(APPLE)
    target_link_options(${PROJECT_NAME} PRIVATE "-Wl,-exported_symbols_list,${CMAKE_CURRENT_SOURCE_DIR}/sym/exported_symbols.lds")
elseif(UNIX)
    target_link_options(${PROJECT_NAME} PRIVATE "-Wl,--version-script,${CMAKE_CURRENT_SOURCE_DIR}/sym/version_script.lds")
endif()

if(PLUGIFY_USE_SANITIZER)
    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>: /fsanitize=address>)
        target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>: _DISABLE_VECTOR_ANNOTATION>)
    else()
        target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>: -fsanitize=address -fsanitize=leak -fsanitize=undefined>)
        target_link_libraries(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>: -fsanitize=address -fsanitize=leak -fsanitize=undefined>)
    endif()
endif()

include(GenerateExportHeader)
generate_export_header(${PROJECT_NAME} EXPORT_MACRO_NAME V8LM_EXPORT EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/exports/module_export.h)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR}/exports)

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
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.pmodule.in
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pmodule
)

set(V8LM_GITHUB_ACTIONS "0" CACHE STRING "Build with github actions")

if(V8LM_GITHUB_ACTIONS)
    execute_process(COMMAND cmake -E create_symlink
            "${CMAKE_CURRENT_SOURCE_DIR}/lib"
            "${CMAKE_CURRENT_BINARY_DIR}/lib"
    )
endif()

