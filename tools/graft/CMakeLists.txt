#C++ Based hook command

if(WIN32)
  set(GRAFT_WIN_SRC
    graft.manifest
    graft.rc
  )
endif()

add_executable(git-graft
  ../cmd_main.cc
  graft.cc
  ${GRAFT_WIN_SRC}
)

if(MINGW)
  target_compile_options(git-graft PRIVATE -municode)
  set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -municode")
  #set_target_properties(git-analyze PROPERTIES COMPILE_FLAGS -municode)
endif()

add_dependencies(git-graft
  libgit2
)

target_link_libraries(git-graft
  absl::str_format
  absl::strings
  ${LIBGIT2_LIBS}
  ${UV_LIBS}
)

install(TARGETS git-graft
    DESTINATION bin
)
