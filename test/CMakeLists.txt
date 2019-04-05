# cmake test

add_executable(aze
  aze.cc
)
add_executable(treewalk
  treewalk.cc
)

if(WIN32)
  target_link_libraries(aze
    ${LIBGIT2_LIBS}
  )
  target_link_libraries(treewalk
    ${LIBGIT2_LIBS}
  )
else()
target_link_libraries(aze
  ${LIBGIT2_LIBS}
  pthread
)
target_link_libraries(treewalk
  ${LIBGIT2_LIBS}
  pthread
)
endif()
