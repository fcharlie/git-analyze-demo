#C++ Based hook command

if(WIN32)
add_executable(git-analyze
  driver.cc
  analyze.cc
  environment.cc
  git-analyze.rc
  git-analyze.manifest
)
else()
add_executable(git-analyze
  driver.cc
  analyze.cc
  environment.cc
)
endif()

if(MINGW)
    target_compile_options(git-analyze PRIVATE -municode)
    set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -municode")
    #set_target_properties(git-analyze PROPERTIES COMPILE_FLAGS -municode)
endif()

target_link_libraries(git-analyze
  Pal
  git2
)

install(TARGETS git-analyze
    DESTINATION bin
)
