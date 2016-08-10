# cmake test

add_executable(aze
  aze.cc
)
add_executable(treewalk
  treewalk.cc
)

if(WIN32)
  target_link_libraries(aze
    git2
  )
  target_link_libraries(treewalk
    git2
  )
else()
target_link_libraries(aze
  git2
  pthread
)
target_link_libraries(treewalk
  git2
  pthread
)
endif()
