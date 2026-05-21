cmake_minimum_required(VERSION 4.1.2)

include (CMake/vcpkg_setup.cmake)

# --------------------- Dependencies ---------------------
# Download CPM.cmake
file(
  DOWNLOAD
  https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.40.8/CPM.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake
  EXPECTED_HASH SHA256=78ba32abdf798bc616bab7c73aac32a17bbd7b06ad9e26a6add69de8f3ae4791
)
include(${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake)
include(CMake/stb.cmake)
  # --------------------- End dependencies ---------------------

message(STATUS "C compiler: ${CMAKE_C_COMPILER}")
message(STATUS "CXX compiler: ${CMAKE_CXX_COMPILER}")

project(BayerImageCreate VERSION 0.1.0 LANGUAGES C CXX)

find_package(TIFF REQUIRED)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Folders for IDEs that support it
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# We need to specify these types since we are supporting multi-config IDEs like Xcode and Visual Studio
set(CMAKE_CONFIGURATION_TYPES "Debug" "Release")

add_executable(BayerImageCreate
  Source/main.cpp
  Source/Image/Bayer.hpp
  Source/Image/JPEG.hpp
  Source/Image/Tiff.hpp
  Source/Macbeth/Macbeth.hpp
  Source/Types/Types.hpp
)

target_include_directories(BayerImageCreate PRIVATE "Source")

target_link_libraries(BayerImageCreate
    PRIVATE
        stb
        TIFF::TIFF
)

# Needed so that when app starts up, it can locate the shared libraries
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
    set(RPATH_OPTION "-Wl,-rpath,/usr/local/gcc-15/lib64")
    if(CMAKE_CXX_COMPILER STREQUAL "/usr/bin/clang++")
      set(RPATH_OPTION "-Wl,-rpath,/usr/lib/llvm-21/lib")
    endif()
    target_link_options(BayerImageCreate
        PRIVATE
        ${RPATH_OPTION}
    )
endif()
