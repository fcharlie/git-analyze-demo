# git-complete

if(WIN32)
  set(COMPLETE_WIN_SRC
    complete.manifest
    complete.rc
  )
endif()

add_executable(git-complete
  ../cmd_main.cc
  complete.cc
  commit.cc
  ${COMPLETE_WIN_SRC}
)

if(MINGW)
  target_compile_options(git-complete PRIVATE -municode)
  set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -municode")
  #set_target_properties(git-analyze PROPERTIES COMPILE_FLAGS -municode)
endif()

add_dependencies(git-complete
  libgit2
)

target_link_libraries(git-complete
  absl::str_format
  absl::strings
  ${LIBGIT2_LIBS}
)

install(TARGETS git-complete
    DESTINATION bin
)
