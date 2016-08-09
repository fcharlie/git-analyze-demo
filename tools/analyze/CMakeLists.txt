#C++ Based hook command

add_executable(git-analyze
  driver.cc
  analyze.cc
  environment.cc
)

target_link_libraries(git-analyze
  git2
)

install(TARGETS git-analyze
    DESTINATION bin
)
