
# CMake module to integrate vcpkg into the build process.
# This module bootstraps vcpkg and sets the CMAKE_TOOLCHAIN_FILE to use vcpkg's toolchain.

# Commands to add vcpkg as a submodule and commit it to the local repository:
# git submodule add https://github.com/Microsoft/vcpkg.git ThirdParty/vcpkg
# git add ThirdParty/vcpkg .gitmodules
# git commit -m "Add vcpkg as submodule in ThirdParty"
# git submodule update --init --recursive

# Command to bootstrap vcpkg and generate the toolchain file:
# cd ThirdParty/vcpkg
# ./bootstrap-vcpkg.sh

# CMake command to configure the project with vcpkg toolchain:
# cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=ThirdParty/vcpkg/scripts/buildsystems/vcpkg.cmake [other flags]

# Command to set up vcpkg json file, and add 1 port
# .\ThirdParty\vcpkg\vcpkg.exe new --application
# vcpkg add port opencv

# Ref
# https://vcpkg.io/en/package/opencv

message(STATUS "Setting up vcpkg")

find_package(Git QUIET)

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(bootstrap_vcpkg_ext "bat")
else()
    set(bootstrap_vcpkg_ext "sh")
endif()

message(STATUS "Checking for vcpkg submodule")
if(GIT_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
    # Check if a known file in the submodule exists
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/vcpkg/bootstrap-vcpkg.${bootstrap_vcpkg_ext}")
        execute_process(
            COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            RESULT_VARIABLE GIT_SUBMOD_RESULT
        )
        if(NOT GIT_SUBMOD_RESULT EQUAL 0)
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}")
        endif()
    endif()
endif()

message(STATUS "Calling bootstrap-vcpkg.${bootstrap_vcpkg_ext}")
execute_process(
        COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/vcpkg/bootstrap-vcpkg.${bootstrap_vcpkg_ext}"
        RESULT_VARIABLE bootstrap_vcpkg_result
)

if(NOT bootstrap_vcpkg_result EQUAL 0)
    message(FATAL_ERROR "Bootstrapping failed with code: ${bootstrap_vcpkg_result}")
else()
    message(STATUS "Bootstrapping succeeded.")
endif()

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE OR CMAKE_TOOLCHAIN_FILE STREQUAL "")
    message(STATUS "CMAKE_TOOLCHAIN_FILE is not set.")
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/vcpkg/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "vcpkg toolchain file")
else()
    if(EXISTS "${CMAKE_TOOLCHAIN_FILE}")
        message(STATUS "CMAKE_TOOLCHAIN_FILE is set to '${CMAKE_TOOLCHAIN_FILE}' and the file exists.")
    else()
        message(FATAL_ERROR "CMAKE_TOOLCHAIN_FILE is set to '${CMAKE_TOOLCHAIN_FILE}', but the file does not exist.")
    endif()
endif()

message(STATUS "vcpkg setup complete. CMAKE_TOOLCHAIN_FILE set to: ${CMAKE_TOOLCHAIN_FILE}")
