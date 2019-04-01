#C++ Based hook command

if(WIN32)
add_executable(git-complete
  ../cmd_main.cc
  complete.cc
  commit.cc
  complete.manifest
  complete.rc
)
else()
add_executable(git-complete
  ../cmd_main.cc
  complete.cc
  commit.cc
  #environment.cc
)
endif()

if(MINGW)
    target_compile_options(git-complete PRIVATE -municode)
    set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -municode")
    #set_target_properties(git-analyze PROPERTIES COMPILE_FLAGS -municode)
endif()

add_dependencies(git-complete
  libgit2
)

target_link_libraries(git-complete
  Pal
  ${LIBGIT2}
)

install(TARGETS git-complete
    DESTINATION bin
)
