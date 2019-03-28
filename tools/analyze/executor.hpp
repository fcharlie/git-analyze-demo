////
#ifndef AZE_EXECUTOR_HPP
#define AZE_EXECUTOR_HPP
#include <chrono>
#include <thread>
#include <string_view>

namespace aze {
class timer {
public:
  template <typename F> void async_wait(F f, int64_t delay) {
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

class executor {
public:
  executor();
  ~executor();
  executor(const executor &) = delete;
  executor &operator=(const executor &) = delete;
  bool initialize(std::string_view repodir, std::string_view rev,
                  int64_t timeout = -1, bool allrefs = false);

private:
};
} // namespace aze

#endif
