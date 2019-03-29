////////
#ifndef PE_EXECUTOR_HPP
#define PE_EXECUTOR_HPP
#include <string_view>
#include <vector>
#include <unordered_set>

namespace aze {
class Executor {
public:
  Executor() = default;
  Executor(const Executor &) = delete;
  Executor &operator=(const Executor &) = delete;
  bool Initialize();
  bool Execute(std::string_view gitdir, std::string_view oldrev,
               std::string_view newrev);

private:
  std::vector<std::string> ke;
  std::unordered_set<std::string_view> emails;
};
} // namespace aze

#endif
