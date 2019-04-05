# pre-commit

if(WIN32)
  set(PCM_WIN_SRC
    pre-commit.manifest
    pre-commit.rc
  )
endif()

add_executable(pre-commit
  pre-commit.cc
  ${PCM_WIN_SRC}
)

if(MINGW)
    target_compile_options(pre-commit PRIVATE -municode)
    set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -municode")
    #set_target_properties(git-analyze PROPERTIES COMPILE_FLAGS -municode)
endif()

add_dependencies(pre-commit
  libgit2
)

target_link_libraries(pre-commit
  absl::str_format
  absl::strings
  ${LIBGIT2_LIBS}
)

install(TARGETS pre-commit
    DESTINATION bin
)
