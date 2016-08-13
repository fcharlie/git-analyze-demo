/*
* analyze.hpp
* git-analyze
* author: Force.Charlie
* Date: 2016.08
* Copyright (C) 2016. OSChina.NET. All Rights Reserved.
*/
#ifndef GIT_ANALYZE_HPP
#define GIT_ANALYZE_HPP
#include <string>
#include <cstdint>

#define MBSIZE (1UL << 20)

#define GIT_ANALYZE_TIMEOUT "GIT_ANALYZE_TIMEOUT"
#define GIT_ANALYZE_LIMITSIZE "GIT_ANALYZE_LIMITSIZE"
#define GIT_ANALYZE_WARNSIZE "GIT_ANALYZE_WARNSIZE"

#define GIT_ANALYZE_TIMEOUT_INFINITE ((std::int32_t)-1)

std::size_t EnvLimitSize();
std::size_t EnvWarnSize();
std::int32_t EnvTimeout();

extern bool g_showcommitter;
extern std::int64_t g_limitsize;
extern std::int64_t g_warnsize;

struct AnalyzeArgs {
  std::int32_t timeout{-1};
  std::string repository;
  std::string ref;
  bool allrefs{false};
  char reserved[7];
};

bool ProcessAnalyzeTask(const AnalyzeArgs &analyzeArgs);

#endif
