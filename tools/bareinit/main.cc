/*
 * main.cc
 * git-init
 * author: Force.Charlie
 * Date: 2017.06
 * Copyright (C) 2019. GITEE.COM. All Rights Reserved.
 */
#include <ctime>
#include <cstdio>
#include <cstring>
#include <string>
#include <git.hpp>
#include <console.hpp>

int cmd_main(int argc, char **argv) {
  if (argc == 1) {
    aze::FPrintF(stderr, "usage: %s path\n", argv[0]);
    return 1;
  }
  git::global_initializer_t gi;
  git_repository *repo{nullptr};
  if (git_repository_init(&repo, argv[1], 1) != 0) {
    const git_error *error = giterr_last();
    aze::FPrintF(stderr, "init bare repository failed: %s\n", error->message);
    return 1;
  }
  git_repository_free(repo);
  aze::FPrintF(stderr, "initialize bare repository %s\n", argv[1]);
  return 0;
}
