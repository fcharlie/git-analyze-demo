/*
* rollback.cc
* git-rollback
* author: Force.Charlie
* Date: 2016.08
* Copyright (C) 2016. OSChina.NET. All Rights Reserved.
*/
#include <git2.h>
#include "rollback.hpp"

RollbackDriver::RollbackDriver() { git_libgit2_init(); }

RollbackDriver::~RollbackDriver() { git_libgit2_shutdown(); }

bool RollbackDriver::RollbackWithCommit(const char *repodir,
                                        const char *refname, const char *hexid,
                                        bool forced) {
  ///
  return true;
}

bool RollbackDriver::RollbackWithRev(const char *repodir, const char *refname,
                                     int rev, bool forced) {
  //
  return true;
}
