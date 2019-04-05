# git-bare init
if(WIN32)
  set(BARE_WIN_SRC
    bareinit.manifest
    bareinit.rc
  )
endif()

add_executable(git-bareinit
  ../cmd_main.cc
  main.cc
  ${BARE_WIN_SRC}
)

if(MINGW)
  target_compile_options(git-bareinit PRIVATE -municode)
  set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -municode")
  #set_target_properties(git-analyze PROPERTIES COMPILE_FLAGS -municode)
endif()

add_dependencies(git-bareinit
  libgit2
)

target_link_libraries(git-bareinit
  absl::str_format
  absl::strings
  ${LIBGIT2_LIBS}
)

install(TARGETS git-bareinit
    DESTINATION bin
)
