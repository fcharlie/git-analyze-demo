/*
* rollback.hpp
* git-rollback
* author: Force.Charlie
* Date: 2016.08
* Copyright (C) 2016. OSChina.NET. All Rights Reserved.
*/
#ifndef GIT_ANALYZE_ROLLBACK_HPP
#define GIT_ANALYZE_ROLLBACK_HPP

class RollbackDriver {
public:
  RollbackDriver();
  ~RollbackDriver();
  // bool Open()
  bool RollbackWithCommit(const char *refname, const char *hexid, bool forced);
  bool RollbackWithRev(const char *refname, int rev, bool forced);

private:
  ////
};

#endif
