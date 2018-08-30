# Copyright (c) 2018 GITEE.COM All Rights Reserved.
cmake_minimum_required (VERSION 3.10)
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

if(NOT DEFINED GITANALYZE_VERSION_MAJOR)
  set(GITANALYZE_VERSION_MAJOR 2)
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

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

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

# Need CMake 3.10 or Later https://cmake.org/cmake/help/v3.10/prop_tgt/CXX_STANDARD.html#prop_tgt:CXX_STANDARD
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
if (MSVC)
  add_compile_options("-D_CRT_SECURE_NO_WARNINGS=1")
else()
  add_compile_options("-g")
  add_compile_options("-Wall")
  add_compile_options("-Wextra")
endif()

if(ADDRESS_SANITIZER)
  add_compile_options("-fsanitize=address")
  add_compile_options("-fno-omit-frame-pointer")
  set(CMAKE_EXE_LINKER_FLAGS "-fsanitize=address ")
endif()

set(ANALYZE_CLEANROOT "${CMAKE_BINARY_DIR}/cleanroot")
include(ExternalProject)

ExternalProject_Add(libgit2
  URL https://github.com/libgit2/libgit2/archive/v0.27.4.tar.gz
  URL_HASH SHA256=0b7ca31cb959ff1b22afa0da8621782afe61f99242bf716c403802ffbdb21d51
  CMAKE_ARGS
    -DCMAKE_PROJECT_libgit2_INCLUDE=${CMAKE_CURRENT_SOURCE_DIR}/cmake/libgit2.cmake
    -DCMAKE_INSTALL_PREFIX:PATH=${ANALYZE_CLEANROOT}
    -DCMAKE_BUILD_TYPE:STRING=Release
    BUILD_COMMAND make
    INSTALL_COMMAND make install
)

include_directories(
  ${ANALYZE_CLEANROOT}/include
  ./include
  ./vendor/libgit2/include
)

link_directories(
  ${ANALYZE_CLEANROOT}/lib
)


if(WIN32)
  install(FILES "${ANALYZE_CLEANROOT}/lib/git2.dll"
    DESTINATION bin
  )
else()
  install(FILES "${ANALYZE_CLEANROOT}/lib/libgit2.so"
    DESTINATION lib
  )
endif()

install(FILES "${ANALYZE_CLEANROOT}/share/libgit2/COPYING"
  DESTINATION share/libgit2
  RENAME copyright
)

add_subdirectory(lib/Pal)
add_subdirectory(tools/analyze)
add_subdirectory(tools/rollback)
add_subdirectory(tools/cheat)
add_subdirectory(tools/pre-commit)
add_subdirectory(tools/complete)
add_subdirectory(tools/git-bareinit)
add_subdirectory(tools/graft)
add_subdirectory(test)
