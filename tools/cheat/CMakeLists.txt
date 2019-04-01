#C++ Based hook command

if(WIN32)
add_executable(git-cheat
  ../cmd_main.cc
  driver.cc
  cheat.cc
  git-cheat.rc
  git-cheat.manifest
)
else()
add_executable(git-cheat
  ../cmd_main.cc
  driver.cc
  cheat.cc
  #environment.cc
)
endif()

if(MINGW)
    target_compile_options(git-cheat PRIVATE -municode)
    set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -municode")
    #set_target_properties(git-analyze PROPERTIES COMPILE_FLAGS -municode)
endif()

add_dependencies(git-cheat
  libgit2
)

target_link_libraries(git-cheat
  Pal
  ${LIBGIT2}
)

install(TARGETS git-cheat
    DESTINATION bin
)
