#C++ Based hook command

if(WIN32)
add_executable(git-rollback
  ../cmd_main.cc
  driver.cc
  rollback.cc
  gc.cc
  git-rollback.rc
  git-rollback.manifest
)
else()
add_executable(git-rollback
  ../cmd_main.cc
  driver.cc
  rollback.cc
  gc.cc
)
endif()



if(MINGW)
    target_compile_options(git-rollback PRIVATE -municode)
    set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -municode")
    #set_target_properties(git-rollback PROPERTIES COMPILE_FLAGS -municode)
endif()

target_link_libraries(git-rollback
  Pal
  ${LIBGIT2}
)

install(TARGETS git-rollback
    DESTINATION bin
)
