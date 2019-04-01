#C++ Based hook command

if(WIN32)
add_executable(git-bareinit
  ../cmd_main.cc
  main.cc
  git-bareinit.manifest
  git-bareinit.rc
)
else()
add_executable(git-bareinit
  ../cmd_main.cc
  main.cc
)
endif()

if(MINGW)
    target_compile_options(git-bareinit PRIVATE -municode)
    set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -municode")
    #set_target_properties(git-analyze PROPERTIES COMPILE_FLAGS -municode)
endif()

target_link_libraries(git-bareinit
  Pal
  ${LIBGIT2}
)

install(TARGETS git-bareinit
    DESTINATION bin
)
