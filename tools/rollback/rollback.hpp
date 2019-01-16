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

struct RollbackTaskArgs {
  std::string gitdir{"."};
  std::string hexid;
  std::string refname{"HEAD"};
  int rev{-1};
  bool forced{false};
};

class RollbackDriver {
public:
  RollbackDriver();
  ~RollbackDriver();
  bool RollbackWithCommit(const char *repodir, const char *refname,
                          const char *hexid, bool forced);
  bool RollbackWithRev(const char *repodir, const char *refname, int rev,
                       bool forced);

private:
  ////
};
bool GitGCInvoke(const std::string &dir, bool forced);
//
#endif
