# git-cheat

if(WIN32)
  set(CHEAT_WIN_SRC
    git-cheat.rc
    git-cheat.manifest
  )
endif()

add_executable(git-cheat
  ../cmd_main.cc
  driver.cc
  cheat.cc
  ${CHEAT_WIN_SRC}
)

add_dependencies(git-cheat
  libgit2
)

if(MINGW)
  target_compile_options(git-cheat PRIVATE -municode)
  set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -municode")
  #set_target_properties(git-analyze PROPERTIES COMPILE_FLAGS -municode)
endif()

target_link_libraries(git-cheat
  absl::str_format
  absl::strings
  absl::time
  ${LIBGIT2_LIBS}
)

install(TARGETS git-cheat
    DESTINATION bin
)
