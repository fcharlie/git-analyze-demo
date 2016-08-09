#C++ Based hook command

if(WIN32)
add_executable(git-analyze
  driver.cc
  analyze.cc
  environment.cc
  git-analyze.rc
)
else()
add_executable(git-analyze
  driver.cc
  analyze.cc
  environment.cc
)
endif()



target_link_libraries(git-analyze
  git2
)

install(TARGETS git-analyze
    DESTINATION bin
)
