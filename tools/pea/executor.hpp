////////
#ifndef PE_EXECUTOR_HPP
#define PE_EXECUTOR_HPP
#include <string_view>
#include <vector>
#include <absl/container/flat_hash_set.h>

namespace aze {
class Executor {
public:
  Executor() = default;
  Executor(const Executor &) = delete;
  Executor &operator=(const Executor &) = delete;
  bool Initialize();
  bool Execute(std::string_view gitdir, std::string_view oldrev,
               std::string_view newrev);
  bool IsBlocked() const { return blocked; }
  // Check emails exists
  bool Exists(std::string_view email) const {
    return emails.find(email) != emails.end();
  }

private:
  absl::flat_hash_set<std::string> emails;
  bool blocked{false};
};
} // namespace aze

#endif
