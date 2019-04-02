/*
 * complete.cc
 * git-analyze
 * author: Force.Charlie
 * Date: 2016.12
 * Copyright (C) 2019. GITEE.COM. All Rights Reserved.
 */
/// to create total year commits
#include "complete.hpp"

//// this is real main
int cmd_main(int argc, char **argv) {
  int start_year = 0;
  int end_year = 0;
  if (argc < 3) {
    Printe("usage: %s  dir branch message year(or year range) day\nExample: "
           "git-complete . dev 'no commit message' 2017~2020 \n",
           argv[0]);
    return 1;
  }
  bool createNewbranch = false;
  if (argc > 5) {
    createNewbranch = (strcmp("--nb", argv[5]) == 0 || strcmp("--NB", argv[5]));
  }
  git::global_initializer_t gi;
  Executor ex;
  if (!ex.Initialize(argv[1], argv[2], argv[3], createNewbranch)) {
    return 1;
  }
  if (argc > 4) {
    char *c = nullptr;
    start_year = strtol(argv[4], &c, 10);
    if (c != nullptr && (*c == '~' || *c == '-')) {
      char *c2 = nullptr;
      end_year = strtol(c + 1, &c2, 10);
    }
    if (end_year == 0) {
      end_year = start_year;
    }
  }
  if (ex.Execute(start_year, end_year)) {
    fprintf(stderr, "\rHas completed in %d to %d's commits !\n", start_year,
            end_year);
  }
  return 0;
}
