# cmake test

add_executable(aze
  aze.cc
)
add_executable(treewalk
  treewalk.cc
)

if(WIN32)
  target_link_libraries(aze
    ${LIBGIT2}
  )
  target_link_libraries(treewalk
    ${LIBGIT2}
  )
else()
target_link_libraries(aze
  ${LIBGIT2}
  pthread
)
target_link_libraries(treewalk
  ${LIBGIT2}
  pthread
)
endif()
