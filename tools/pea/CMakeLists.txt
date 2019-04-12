
add_executable(pea
  ../cmd_main.cc
  driver.cc
  executor.cc
)

if(MINGW)
  target_compile_options(pea PRIVATE -municode)
  set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -municode")
  #set_target_properties(git-analyze PROPERTIES COMPILE_FLAGS -municode)
endif()

target_link_libraries(pea
  ${LIBGIT2_LIBS}
  absl::container
  absl::flat_hash_map
  absl::hash
  absl::str_format
  absl::strings
  ${UV_LIBS}
)
