# Copyright (c) 2019 GITEE.COM All Rights Reserved.

cmake_minimum_required (VERSION 3.12)
project(git-analyze)


if( CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR AND NOT MSVC_IDE )
  message(FATAL_ERROR "In-source builds are not allowed.
CMake would overwrite the makefiles distributed with GitHook.
Please create a directory and run cmake from there, passing the path
to this source directory as the last argument.
This process created the file `CMakeCache.txt' and the directory `CMakeFiles'.
Please delete them.")
endif()

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "No build type selected, default to Debug")
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type (default Debug)" FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()


if(NOT DEFINED GITANALYZE_VERSION_MAJOR)
  set(GITANALYZE_VERSION_MAJOR 3)
endif()
if(NOT DEFINED GITANALYZE_VERSION_MINOR)
  set(GITANALYZE_VERSION_MINOR 0)
endif()
if(NOT DEFINED GITANALYZE_VERSION_PATCH)
  set(GITANALYZE_VERSION_PATCH 0)
endif()

if (POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
  set(cmake_3_0_PROJ_VERSION
    VERSION ${GITANALYZE_VERSION_MAJOR}.${GITANALYZE_VERSION_MINOR}.${GITANALYZE_VERSION_PATCH})
  set(cmake_3_0_LANGUAGES LANGUAGES)
endif()

if (NOT PACKAGE_VERSION)
  set(PACKAGE_VERSION
    "${GITANALYZE_VERSION_MAJOR}.${GITANALYZE_VERSION_MINOR}.${GITANALYZE_VERSION_PATCH}")
endif()

set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")

set(CPACK_PACKAGE_NAME "git-analyze")
set(CPACK_PACKAGE_VENDOR "GITEE.COM")
set(CPACK_PACKAGE_DESCRIPTION "This is git analyze tools")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "GIT Analyze")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "git-analyze")
set(CPACK_PACKAGE_VERSION_MAJOR ${GITANALYZE_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${GITANALYZE_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${GITANALYZE_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION ${PACKAGE_VERSION})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_DESCRIPTION "Git Analyze")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.3.1-6)")
set(CPACK_PACKAGE_CONTACT "admin@oschina.cn")
set(CPACK_DEBIAN_PACKAGE_SECTION T)
if(WIN32 AND NOT UNIX)
  set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "GitAnalyze")
  set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}\\\\cmake\\\\git.ico")
  set(CPACK_NSIS_MUI_UNIICON "${CMAKE_CURRENT_SOURCE_DIR}\\\\cmake\\\\git.ico")
  set(CPACK_NSIS_MODIFY_PATH "ON")
  set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL "ON")
  if( CMAKE_CL_64 )
    set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
  endif()
endif()

include(CPack)

if(WIN32)
  include(InstallRequiredSystemLibraries)
endif()

option(USE_SANITIZE "Use address sanitizer compile git-analyze" OFF)
option(ENABLE_AZE_CLAR "Enable build aze test utils" OFF)

message(STATUS "Build git-analyze on ${CMAKE_SYSTEM_NAME} ")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

# Need CMake 3.10 or Later https://cmake.org/cmake/help/v3.10/prop_tgt/CXX_STANDARD.html#prop_tgt:CXX_STANDARD
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

if (MSVC)
  add_compile_options("-D_CRT_SECURE_NO_WARNINGS=1")
  add_compile_options("-Zc:__cplusplus") # C++ __cplusplus
  set(WIN_COMPILER_FLAGS
    CMAKE_CXX_FLAGS
    CMAKE_CXX_FLAGS_DEBUG
    CMAKE_CXX_FLAGS_RELEASE
    CMAKE_C_FLAGS
    CMAKE_C_FLAGS_DEBUG
    CMAKE_C_FLAGS_RELEASE
  )
  foreach(WIN_COMPILER_FLAG ${WIN_COMPILER_FLAGS})
    string(REPLACE "/MD" "/MT" ${WIN_COMPILER_FLAG} "${${WIN_COMPILER_FLAG}}")
  endforeach()
else()
  add_compile_options("-g")
  add_compile_options("-Wall")
  add_compile_options("-Wextra")
  if(USE_SANITIZE)
    add_compile_options("-fsanitize=address")
    add_compile_options("-fno-omit-frame-pointer")
    set(CMAKE_EXE_LINKER_FLAGS "-fsanitize=address ")
  endif()
endif()

set(ANALYZE_CLEANROOT "${CMAKE_BINARY_DIR}/cleanroot")

message(STATUS "TRACE '${CMAKE_GENERATOR}' ${CMAKE_MAKE_PROGRAM}")

include(ExternalProject)
include(FetchContent) #cmake 3.11 or Later

#Absl
set(ABSL_REPO https://github.com/abseil/abseil-cpp.git)
set(ABSL_TAG e6b050212c859fbaf67abac76105da10ec348274)

# Libgit2
set(LIBGIT2_URL https://github.com/libgit2/libgit2/archive/v0.28.2.tar.gz)
set(LIBGIT2_HASH 42b5f1e9b9159d66d86fff0394215c5733b6ef8f9b9d054cdd8c73ad47177fc3)
# HTTP Parser
set(HTTP_PARSER_URL https://github.com/nodejs/http-parser/archive/v2.9.2.tar.gz)
set(HTTP_PARSER_HASH 5199500e352584852c95c13423edc5f0cb329297c81dd69c3c8f52a75496da08)
# libre2
set(LIBRE2_URL https://github.com/google/re2/archive/2019-06-01.tar.gz)
set(LIBRE2_HASH 02b7d73126bd18e9fbfe5d6375a8bb13fadaf8e99e48cbb062e4500fc18e8e2e)
set(LIBUV_URL https://github.com/libuv/libuv/archive/v1.30.1.tar.gz)
set(LIBUV_HASH d85566c2c4eae7d8e2c2d27d40e728fea29f9086e98e795c5cdce1a790f43de5)

set(LIBGIT2_LIBS git2)
set(AZE_UNIX_LIB)
set(AZE_BUILD_TYPE Release)

if(WIN32)
  set(AZE_BUILD_TYPE ${CMAKE_BUILD_TYPE})
  # Build libgit2 static
  ExternalProject_Add(libgit2
    URL ${LIBGIT2_URL}
    URL_HASH SHA256=${LIBGIT2_HASH}
    CMAKE_ARGS
      -G${CMAKE_GENERATOR}
      -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
      -DBUILD_SHARED_LIBS=OFF
      -DSTATIC_CRT=ON
      -DCMAKE_PROJECT_libgit2_INCLUDE=${CMAKE_CURRENT_SOURCE_DIR}/cmake/libgit2.cmake
      -DCMAKE_INSTALL_PREFIX:PATH=${ANALYZE_CLEANROOT}
      -DCMAKE_BUILD_TYPE:STRING=${AZE_BUILD_TYPE}
      BUILD_COMMAND cmake --build .
      INSTALL_COMMAND ${CMAKE_MAKE_PROGRAM} install
  )
  set(LIBGIT2_LIBS
    git2
    ws2_32
    winhttp
    rpcrt4
    crypt32
    ole32
  )
  set(RE2_WIN_OPT
    -DCRT_FLAG_DEBUG=${CRT_FLAG_DEBUG}
    -DCRT_FLAG_RELEASE=${CRT_FLAG_RELEASE}
  )

else()
  ExternalProject_Add(http-parser
    URL ${HTTP_PARSER_URL}
    URL_HASH SHA256=${HTTP_PARSER_HASH}
    PATCH_COMMAND patch -Nbp1 -i ${CMAKE_CURRENT_SOURCE_DIR}/patch/http-parser.patch
    CMAKE_ARGS
      -G${CMAKE_GENERATOR}
      -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
      -DCMAKE_INSTALL_PREFIX:PATH=${ANALYZE_CLEANROOT}
      -DCMAKE_BUILD_TYPE:STRING=${AZE_BUILD_TYPE}
    BUILD_COMMAND cmake --build .
    INSTALL_COMMAND ${CMAKE_MAKE_PROGRAM} install
  )
  # Build libgit2 shared
  ExternalProject_Add(libgit2
    DEPENDS http-parser
    URL ${LIBGIT2_URL}
    URL_HASH SHA256=${LIBGIT2_HASH}
    CMAKE_ARGS
      -G${CMAKE_GENERATOR}
      -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
      -DCMAKE_PROJECT_libgit2_INCLUDE=${CMAKE_CURRENT_SOURCE_DIR}/cmake/libgit2.cmake
      -DCMAKE_INSTALL_PREFIX:PATH=${ANALYZE_CLEANROOT}
      -DCMAKE_BUILD_TYPE:STRING=${AZE_BUILD_TYPE}
      BUILD_COMMAND cmake --build .
      INSTALL_COMMAND ${CMAKE_MAKE_PROGRAM} install
  )
  install(FILES "${ANALYZE_CLEANROOT}/lib/libgit2.so"
    DESTINATION lib
  )
  set(LIBGIT2_LIBS
    git2
    z
    http_parser
  )
  set(AZE_UNIX_LIB pthread)
endif()

if(NOT WIN32)
# build libuv
ExternalProject_Add(libuv
  URL ${LIBUV_URL}
  URL_HASH SHA256=${LIBUV_HASH}
  CMAKE_ARGS
    -G${CMAKE_GENERATOR}
    -DCMAKE_BUILD_TYPE:STRING=${AZE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX:PATH=${ANALYZE_CLEANROOT}
    -DBUILD_SHARED_LIBS=OFF
    -DBUILD_TESTING=OFF
    BUILD_COMMAND cmake --build .
    INSTALL_COMMAND ${CMAKE_MAKE_PROGRAM} install
)
else()
  FetchContent_Declare(libuv
    URL ${LIBUV_URL}
    URL_HASH SHA256=${LIBUV_HASH}
  )
  FetchContent_GetProperties(libuv)
  if(NOT libuv_POPULATED)
    message(STATUS "Fetch content libuv ...")
    FetchContent_Populate(libuv)
    message(STATUS "Add include dir ${libuv_SOURCE_DIR}/include ...")
    include_directories(
      "${libuv_SOURCE_DIR}/include"
    )
  add_subdirectory("${libuv_SOURCE_DIR}")
  endif()
endif()

# build libre2
ExternalProject_Add(libre2
  URL ${LIBRE2_URL}
  URL_HASH SHA256=${LIBRE2_HASH}
  CMAKE_ARGS
    -G${CMAKE_GENERATOR}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
    -DCMAKE_CXX_FLAGS_RELEASE=${CMAKE_CXX_FLAGS_RELEASE}
    -DCMAKE_CXX_FLAGS_DEBUG=${CMAKE_CXX_FLAGS_DEBUG}
    -DCMAKE_CXX_STANDARD=17
    -DCMAKE_BUILD_TYPE:STRING=${AZE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX:PATH=${ANALYZE_CLEANROOT}
    -DBUILD_SHARED_LIBS=OFF
    -DRE2_BUILD_TESTING=OFF
    BUILD_COMMAND cmake --build .
    INSTALL_COMMAND ${CMAKE_MAKE_PROGRAM} install
)

set(UV_LIBS
  uv_a
)


FetchContent_Declare(absl
  GIT_REPOSITORY ${ABSL_REPO}
  GIT_TAG ${ABSL_TAG}
)

FetchContent_GetProperties(absl)
if(NOT absl_POPULATED)
  message(STATUS "Fetch content absl ${ABSL_TAG} ...")
  FetchContent_Populate(absl)
endif()

option(BUILD_TESTING "Force Disable Abseil Test" OFF)
add_subdirectory("${absl_SOURCE_DIR}")


include_directories(
  ${ANALYZE_CLEANROOT}/include
  ${absl_SOURCE_DIR}
  ./include
)

link_directories(
  ${ANALYZE_CLEANROOT}/lib
)

install(FILES "${ANALYZE_CLEANROOT}/share/libgit2/COPYING"
  DESTINATION share/libgit2
  RENAME copyright
)

add_subdirectory(tools) # add tools
if(ENABLE_AZE_CLAR)
  add_subdirectory(test) # test
endif()
