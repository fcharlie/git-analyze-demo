#C++ Based hook command

if(WIN32)
add_executable(git-analyze
  ../cmd_main.cc
  driver.cc
  executor.cc
  git-analyze.rc
  git-analyze.manifest
)
else()
add_executable(git-analyze
  ../cmd_main.cc
  driver.cc
  executor.cc
)
endif()

add_dependencies(git-analyze
  libgit2
)

if(MINGW)
    target_compile_options(git-analyze PRIVATE -municode)
    set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -municode")
endif()

IF(WIN32)
  target_link_libraries(git-analyze
    Pal
    ${LIBGIT2}
  )
else()
  target_link_libraries(git-analyze
    Pal
    ${LIBGIT2}
    pthread
  )
ENDIF()



install(TARGETS git-analyze
    DESTINATION bin
)
