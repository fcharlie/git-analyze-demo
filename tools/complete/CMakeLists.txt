#C++ Based hook command

if(WIN32)
add_executable(git-complete
  complete.cc
  complete.manifest
  complete.rc
)
else()
add_executable(git-complete
  complete.cc
  #environment.cc
)
endif()

if(MINGW)
    target_compile_options(git-complete PRIVATE -municode)
    set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -municode")
    #set_target_properties(git-analyze PROPERTIES COMPILE_FLAGS -municode)
endif()

target_link_libraries(git-complete
  Pal
  git2
)

install(TARGETS git-complete
    DESTINATION bin
)
