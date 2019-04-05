option(SONAME "" OFF)
option(USE_SSH "" OFF)
option(USE_HTTPS "" OFF)
option(USE_GSSAPI "" OFF)
option(BUILD_CLAR "Build clar" OFF)
option(ENABLE_TEST "Build Test" OFF)

if(NOT WIN32)
  set(HTTP_PARSER_INCLUDE_DIR "${CMAKE_INSTALL_PREFIX}/include")
  set(HTTP_PARSER_LIBRARY "${CMAKE_INSTALL_PREFIX}/lib")
endif()

install(FILES COPYING
  DESTINATION share/libgit2
)
