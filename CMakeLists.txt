# Copyright (c) 2016 OSChina.NET All Rights Reserved.
# CMake Required version
cmake_minimum_required (VERSION 2.8)
project(git-analyze)


if( CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR AND NOT MSVC_IDE )
  message(FATAL_ERROR "In-source builds are not allowed.
CMake would overwrite the makefiles distributed with GitHook.
Please create a directory and run cmake from there, passing the path
to this source directory as the last argument.
This process created the file `CMakeCache.txt' and the directory `CMakeFiles'.
Please delete them.")
endif()

OPTION(ADDRESS_SANITIZER "Enable Address Sanitizer, When debug" OFF)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "No build type selected, default to Debug")
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type (default Debug)" FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

message(STATUS "Build git-analyze on ${CMAKE_SYSTEM_NAME} ")


set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)


set(CMAKE_INSTALL_PREFIX "/usr/local")

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

set(CPACK_PACKAGE_NAME "git-analyze")
set(CPACK_PACKAGE_VENDOR "OSChina.NET")
set(CPACK_PACKAGE_DESCRIPTION "This is git analyze tools")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "GIT Analyze")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "_CPack_")
set(CPACK_PACKAGE_VERSION "1.0.0.0")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.3.1-6)")
set(CPACK_PACKAGE_DESCRIPTION "Git Analyze")
set(CPACK_PACKAGE_CONTACT "ipvb@qq.com")
set(CPACK_DEBIAN_PACKAGE_SECTION Network)
include(CPack)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX14)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
if(COMPILER_SUPPORTS_CXX14)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
elseif(COMPILER_SUPPORTS_CXX11)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

if (NOT MSVC)
    add_compile_options("-g")
    add_compile_options("-Wall")
    add_compile_options("-Wextra")
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
  add_compile_options("-DDEBUG=1")
  message(STATUS "DEBUG build.")
elseif(CMAKE_BUILD_TYPE MATCHES Release)
  message(STATUS "${CMAKE_BUILD_TYPE} build.")
  add_compile_options("-DNODEBUG=1")
elseif(CMAKE_BUILD_TYPE MATCHES MinSizeRel)
  message(STATUS "${CMAKE_BUILD_TYPE} build.")
  add_compile_options("-DNODEBUG=1")
else()
   message(STATUS "Add -DDEBUG flags.")
   add_compile_options("-DDEBUG=1")
endif()


if(ADDRESS_SANITIZER)
  add_compile_options("-fsanitize=address")
  add_compile_options("-fno-omit-frame-pointer")
  set(CMAKE_EXE_LINKER_FLAGS "-fsanitize=address ")
endif()

include_directories(
    ./include
    ./vendor/libgit2/include
)



add_subdirectory(vendor/libgit2)
add_subdirectory(test)
