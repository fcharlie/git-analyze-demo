
add_executable(pea
  ../cmd_main.cc
  driver.cc
  executor.cc
)


target_link_libraries(pea
  ${LIBGIT2}
  absl::container
  absl::flat_hash_map
  absl::hash
  absl::str_format
  absl::strings
)
