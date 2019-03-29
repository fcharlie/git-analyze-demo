// private email checker
#include <git.hpp>
#include "executor.hpp"

// check commit emails exists in private-email set.
namespace aze {
bool Executor::Initialize() {
  //
  return true;
}

bool Executor::Execute(std::string_view gitdir, std::string_view oldrev,
                       std::string_view newrev) {
  constexpr const std::string_view zid =
      "0000000000000000000000000000000000000000";
  if (newrev == zid) {
    // delete branch not need check
    return true;
  }
  git::error_code ec;
  auto r = git::repository::make_repository(gitdir, ec);
  if (!r) {
    fprintf(stderr, "unable open repo\n");
    return false;
  }
  //

  return true;
}
} // namespace aze
