#C++ Based hook command

if(WIN32)
add_executable(git-rollback
  driver.cc
  rollback.cc
  gc.cc
  git-rollback.rc
  git-rollback.manifest
)
else()
add_executable(git-rollback
  driver.cc
  rollback.cc
  gc.cc
)
endif()



target_link_libraries(git-rollback
  Pal
  git2
)

install(TARGETS git-rollback
    DESTINATION bin
)
