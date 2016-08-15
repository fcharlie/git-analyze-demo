# cmake test

add_executable(aze
  aze.cc
)
add_executable(treewalk
  treewalk.cc
)

add_executable(palenv
  palenv.cc
)

  target_link_libraries(palenv
    Pal
    git2
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
