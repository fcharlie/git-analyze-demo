#C++ Based hook command

if(WIN32)
add_executable(git-graft
  ../cmd_main.cc
  graft.cc
)
else()
add_executable(git-graft
  ../cmd_main.cc
  graft.cc
)
endif()

if(MINGW)
    target_compile_options(git-graft PRIVATE -municode)
    set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -municode")
    #set_target_properties(git-analyze PROPERTIES COMPILE_FLAGS -municode)
endif()

target_link_libraries(git-graft
  git2
)

install(TARGETS git-graft
    DESTINATION bin
)
