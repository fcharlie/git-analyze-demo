////////
#ifndef PE_EXECUTOR_HPP
#define PE_EXECUTOR_HPP
#include <string_view>
#include <unordered_set>

namespace aze {
class Executor {
public:
  Executor() = default;
  Executor(const Executor &) = delete;
  Executor &operator=(const Executor &) = delete;

private:
};
} // namespace aze

#endif
