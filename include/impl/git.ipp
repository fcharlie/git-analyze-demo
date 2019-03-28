////////
#ifndef AZE_IMPL_GIT_IPP
#define AZE_IMPL_GIT_IPP
#include "git.hpp"

namespace git {

inline repository::repository(repository &&other) {
  if (repo_ != nullptr) {
    git_repository_free(repo_);
  }
  repo_ = other.repo_;
  other.repo_ = nullptr;
}

inline std::optional<repository>
repository::make_repository(std::string_view sv, error_code &ec) {
  repository repo;
  if ((ec.ec = git_repository_open(&repo.repo_, sv.data())) != 0) {
    auto e = giterr_last();
    if (e != nullptr) {
      ec.message = e->message;
    }
    return std::nullopt;
  }
  return std::make_optional(std::move(repo));
}
} // namespace git

#endif
