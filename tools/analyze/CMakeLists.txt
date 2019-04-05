#C++ Based hook command

if(WIN32)
  set(AZE_WIN_SRC
    git-analyze.rc
    git-analyze.manifest
  )
endif()

add_executable(git-analyze
  ../cmd_main.cc
  driver.cc
  executor.cc
  ${AZE_WIN_SRC}
)

add_dependencies(git-analyze
  libgit2
)

target_link_libraries(git-analyze
  absl::str_format
  absl::strings
  ${LIBGIT2_LIBS}
  ${AZE_UNIX_LIB}
)

if(MINGW)
    target_compile_options(git-analyze PRIVATE -municode)
    set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -municode")
endif()

install(TARGETS git-analyze
    DESTINATION bin
)
