#

add_executable(update
  ../cmd_main.cc
  driver.cc
  engine.cc
  executor.cc
)

if(MINGW)
    target_compile_options(update PRIVATE -municode)
    set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -municode")
    #set_target_properties(git-analyze PROPERTIES COMPILE_FLAGS -municode)
endif()

target_link_libraries(update
  ${LIBGIT2_LIBS}
  re2
  absl::str_format
  absl::strings
  ${AZE_UNIX_LIB}
)

add_executable(update_test
  update_test.cc
  engine.cc
)

add_dependencies(update
  libgit2
  libre2
)

target_link_libraries(update_test
  re2
  ${AZE_UNIX_LIB}
)

install(TARGETS update
    DESTINATION bin
)

configure_file(
  "rw.json"
  "${CMAKE_BINARY_DIR}/bin/rw.json" COPYONLY
)
