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

inline std::optional<reference>
repository::get_reference(std::string_view ref) {
  reference r;
  if (git_reference_lookup(&r.ref_, repo_, ref.data()) == 0) {
    return std::make_optional(std::move(r));
  }
  return std::nullopt;
}
inline std::optional<reference>
repository::get_branch(std::string_view branch) {
  reference r;
  if (git_branch_lookup(&r.ref_, repo_, branch.data(), GIT_BRANCH_LOCAL) == 0) {
    return std::make_optional(std::move(r));
  }
  return std::nullopt;
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
