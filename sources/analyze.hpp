///

#ifndef GIT_ANALYZE_HPP
#define GIT_ANALYZE_HPP
#include <cstdint>

/// CreateTimerQueueTimer
/// timer
class AnalyzeTask {
public:
  AnalyzeTask(const char *repodir, const char *branch);
  AnalyzeTask(const AnalyzeTask &) = delete;
  AnalyzeTask &operator=(const AnalyzeTask &) = delete;
  std::size_t &LimitSize() { return limitsize_; }
  std::size_t &WarnSize() { return warnsize_; }
  ///
  // Timeout task
  ///
  std::int64_t &Timeout() { return timeout_; }

private:
  std::int64_t timeout_ = -1; /// when timeout_=-1 not set
  std::size_t limitsize_;
  std::size_t warnsize_;
};

#endif
