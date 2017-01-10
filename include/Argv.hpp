/*
* Argv.hpp
* GIT Analyze Argv Parse
* author: Force.Charlie
* Date: 2016.08
* Copyright (C) 2017. OSChina.NET. All Rights Reserved.
*/

#ifndef GIT_ANALYZE_ARGV_HPP
#define GIT_ANALYZE_ARGV_HPP
#include <cstdint>
#include <cstring>

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

inline bool IsArg(const char *candidate, const char *longname, size_t n,
                  const char **off) {
  auto l = strlen(candidate);
  if (l < n)
    return false;
  if (strncmp(candidate, longname, n) == 0) {
    if (l > n && candidate[n] == '=') {
      *off = candidate + n + 1;
    } else {
      *off = nullptr;
    }
    return true;
  }
  return false;
}

#endif
