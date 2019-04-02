////
#ifndef AZE_EXECUTOR_HPP
#define AZE_EXECUTOR_HPP
#include <chrono>
#include <thread>
#include <string_view>
#include <git.hpp>

namespace aze {
class timer {
public:
  template <typename F> void async_wait(int64_t delay, F f) {
    std::thread t([=]() {
      if (this->clear) {
        return;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(delay));
      if (this->clear) {
        return;
      }
      f();
    });
    t.detach();
  }

private:
  bool clear;
};

class Executor {
public:
  Executor(std::uint64_t wsize, std::uint64_t lsize) : ws(wsize), ls(lsize) {}
  ~Executor() = default;
  Executor(const Executor &) = delete;
  Executor &operator=(const Executor &) = delete;
  bool Initialize(std::string_view gitdir);
  bool AzeAll(std::int64_t timeout = -1);
  bool AzeOne(std::string_view refname, std::int64_t timeout = -1);
  std::uint64_t WarnSize() const { return ws; }
  std::uint64_t LargeSize() const { return ls; }
  bool ShowCommiter() const { return showcommitter; }

private:
  bool AzeOneInternal(const git_oid *id);
  git::repository r;
  timer t;
  std::uint64_t ws;
  std::uint64_t ls;
  bool showcommitter{false};
};
} // namespace aze

#endif
