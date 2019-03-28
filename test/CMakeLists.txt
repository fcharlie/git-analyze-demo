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
    ${LIBGIT2}
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
