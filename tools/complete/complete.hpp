/*
 * complete.hpp
 * git-analyze
 * author: Force.Charlie
 * Date: 2017.04
 * Copyright (C) 2019. GITEE.COM. All Rights Reserved.
 */
#ifndef COMPLETE_HPP
#define COMPLETE_HPP
#include <ctime>
#include <cstdio>
#include <cstring>
#include <string>
#include <string_view>
#include <git.hpp>
#include <console.hpp>

inline unsigned Days(unsigned year) {
  if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)
    return 366;
  return 365;
}

class Executor {
public:
  Executor() = default;
  Executor(const Executor &) = delete;
  Executor &operator=(const Executor &) = delete;
  ~Executor() {
    if (parent != nullptr) {
      git_commit_free(parent);
    }
  }
  bool Initialize(std::string_view dir, std::string_view branch,
                  std::string_view msg, bool nb_);
  bool Execute(uint32_t begin, uint32_t end);

private:
  bool Parseconfig();
  bool RoundYear(int year);
  bool One(git_time when);
  git::repository r;
  git::tree t; // commit tree
  std::string email;
  std::string author;
  std::string msg{"no commit message"};
  std::string refname;
  git_commit *parent{nullptr};
  uint32_t maxcount{7};
  bool nb{false};
};

#endif
