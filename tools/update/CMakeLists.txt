#

add_executable(update
  driver.cc
  engine.cc
  executor.cc
)

if(WIN32)
  target_link_libraries(update
    ${LIBGIT2}
    re2
  )
else()
  target_link_libraries(update
    ${LIBGIT2}
    re2
    pthread
  )
endif()


add_executable(update_test
  update_test.cc
  engine.cc
)
if(WIN32)
  target_link_libraries(update_test
    re2
  )
else()
  target_link_libraries(update_test
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
