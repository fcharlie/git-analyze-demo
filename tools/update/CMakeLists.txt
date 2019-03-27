#

add_executable(update
  driver.cc
  engine.cc
  executor.cc
)

if(WIN32)
  target_link_libraries(update
    git2
    re2
  )
else()
  target_link_libraries(update
    git2
    re2
    pthread
  )
endif()


install(TARGETS update
    DESTINATION bin
)

configure_file(
  "rw.json"
  "${CMAKE_BINARY_DIR}/bin/rw.json" COPYONLY
)
