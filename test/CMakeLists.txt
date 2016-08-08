# cmake test

add_executable(aze
  aze.cc
)

if(WIN32)
  target_link_libraries(aze
    git2
  )
else()
target_link_libraries(aze
  git2
  pthread
)

endif()
