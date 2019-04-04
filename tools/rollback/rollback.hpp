/*
 * rollback.hpp
 * git-rollback
 * author: Force.Charlie
 * Date: 2016.08
 * Copyright (C) 2019. GITEE.COM. All Rights Reserved.
 */
#ifndef GIT_ANALYZE_ROLLBACK_HPP
#define GIT_ANALYZE_ROLLBACK_HPP
#include <string>
#include <string_view>

struct rollback_options {
  std::string gitdir{"."};
  std::string oid;
  std::string refname{"HEAD"};
  int rev{-1};
  bool forced{false};
  bool verbose{false};
};

class Executor {
public:
  Executor() = default;
  Executor(const Executor &) = delete;
  Executor &operator=(const Executor &) = delete;
  rollback_options &options() { return opt_; }
  const rollback_options &options() const { return opt_; }
  bool Execute();

private:
  rollback_options opt_;
};

#endif
