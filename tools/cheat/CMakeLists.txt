#C++ Based hook command

if(WIN32)
add_executable(git-cheat
  cheat.cc
)
else()
add_executable(git-cheat
  cheat.cc
  #environment.cc
)
endif()

if(MINGW)
    target_compile_options(git-cheat PRIVATE -municode)
    set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -municode")
    #set_target_properties(git-analyze PROPERTIES COMPILE_FLAGS -municode)
endif()

target_link_libraries(git-cheat
  Pal
  git2
)

install(TARGETS git-cheat
    DESTINATION bin
)
