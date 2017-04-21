/*
* complete.hpp
* git-analyze
* author: Force.Charlie
* Date: 2017.04
* Copyright (C) 2017. OSChina.NET. All Rights Reserved.
*/
#ifndef COMPLETE_HPP
#define COMPLETE_HPP
#include <ctime>
#include <cstdio>
#include <cstring>
#include <string>
#include <git2.h>
#include <Pal.hpp>
inline unsigned Days(unsigned year) {
  if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)
    return 366;
  return 365;
}

class Demolisher {
private:
  std::string refs;
  std::string message{"no commit message"};
  char name[256];
  char email[256];
  git_repository *repo{nullptr};
  git_commit *current{nullptr};
  bool InitializeRoot();
  bool Userinfo();
  bool CommitBuilder(git_time when, git_commit *parent);
  bool RoundYear(int year, git_commit *parent);

public:
  Demolisher() { git_libgit2_init(); }
  ~Demolisher() {
    if (current) {
      git_commit_free(current);
    }
    if (repo) {
      git_repository_free(repo);
    }
    git_libgit2_shutdown();
  }
  Demolisher(const Demolisher &) = delete;
  Demolisher &operator=(const Demolisher &) = delete;
  bool Initialize(const char *dir, const char *ref,
                  const char *messageTemplate);
  bool IntervalFill(unsigned sy, unsigned ey, bool newref);
};
#endif
