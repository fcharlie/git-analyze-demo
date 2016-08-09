/*
* Argv.hpp
* GIT Analyze Argv Parse
* author: Force.Charlie
* Date: 2016.08
* Copyright (C) 2016. OSChina.NET. All Rights Reserved.
*/

#ifndef GIT_ANALYZE_ARGV_HPP
#define GIT_ANALYZE_ARGV_HPP
#include <cstdint>
#include <cstring>

#ifdef ARGV_NO_LINK

inline bool IsArg(const char *candidate, const char *longname) {
  if (strcmp(candidate, longname) == 0)
    return true;
  return false;
}

inline bool IsArg(const char *candidate, const char *shortname,
                  const char *longname) {
  if (strcmp(candidate, shortname) == 0 ||
      (longname != nullptr && strcmp(candidate, longname) == 0))
    return true;
  return false;
}

#endif
///

#endif
