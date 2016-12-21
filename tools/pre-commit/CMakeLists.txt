#C++ Based hook command

if(WIN32)
add_executable(pre-commit
  pre-commit.cc
  pre-commit.manifest
  pre-commit.rc
)
else()
add_executable(pre-commit
  pre-commit.cc
  #environment.cc
)
endif()

if(MINGW)
    target_compile_options(pre-commit PRIVATE -municode)
    set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -municode")
    #set_target_properties(git-analyze PROPERTIES COMPILE_FLAGS -municode)
endif()

target_link_libraries(pre-commit
  Pal
  git2
)

install(TARGETS pre-commit
    DESTINATION bin
)
