///

#ifndef GIT_ANALYZE_HPP
#define GIT_ANALYZE_HPP
#include <string>
#include <cstdint>

#define MBSIZE (1UL << 20)

#define GIT_ANALYZE_TIMEOUT "GIT_ANALYZE_TIMEOUT"
#define GIT_ANALYZE_LIMITSIZE "GIT_ANALYZE_LIMITSIZE"
#define GIT_ANALYZE_WARNSIZE "GIT_ANALYZE_WARNSIZE"

std::size_t EnvLimitSize();
std::size_t EnvWarnSize();
std::int64_t EnvTimeout();

struct AnalyzeArgs {
  std::size_t limitsize{0};
  std::size_t warnsize{0};
  std::int64_t timeout{-1};
  std::string repository;
  std::string ref;
  bool allrefs{false};
  char reserved[7];
};

/// CreateTimerQueueTimer
/// timer
// class AnalyzeTask {
// public:
//   AnalyzeTask(const char *repodir, const char *branch);
//   AnalyzeTask(const AnalyzeTask &) = delete;
//   AnalyzeTask &operator=(const AnalyzeTask &) = delete;
//   std::size_t &LimitSize() { return limitsize_; }
//   std::size_t &WarnSize() { return warnsize_; }
//   ///
//   // Timeout task
//   ///
//   std::int64_t &Timeout() { return timeout_; }
//
// private:
//   std::int64_t timeout_ = -1; /// when timeout_=-1 not set
//   std::size_t limitsize_;
//   std::size_t warnsize_;
// };

#endif
