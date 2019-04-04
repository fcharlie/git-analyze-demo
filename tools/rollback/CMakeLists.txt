#C++ Based hook command

if(WIN32)
add_executable(git-rollback
  ../cmd_main.cc
  driver.cc
  rollback.cc
  git-rollback.rc
  git-rollback.manifest
)
else()
add_executable(git-rollback
  ../cmd_main.cc
  driver.cc
  rollback.cc
)
endif()



if(MINGW)
    target_compile_options(git-rollback PRIVATE -municode)
    set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -municode")
    #set_target_properties(git-rollback PROPERTIES COMPILE_FLAGS -municode)
endif()


add_dependencies(git-rollback
  libgit2
)

target_link_libraries(git-rollback
  absl::str_format
  absl::strings
  ${LIBGIT2}
)

install(TARGETS git-rollback
    DESTINATION bin
)
