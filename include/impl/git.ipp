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
repository::get_reference(std::string_view refname) {
  reference r;
  if (git_reference_lookup(&r.ref_, repo_, refname.data()) == 0) {
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

inline std::optional<commit> repository::get_commit(std::string_view sid) {
  git_oid oid;
  if (git_oid_fromstrn(&oid, sid.data(), sid.size()) != 0) {
    return std::nullopt;
  }
  commit c;
  if (git_commit_lookup(&c.c, repo_, &oid) != 0) {
    return std::nullopt;
  }
  return std::make_optional(std::move(c));
}

inline std::optional<commit>
repository::get_reference_commit(std::string_view refname) {
  auto ref = get_reference(refname);
  if (!ref) {
    return std::nullopt;
  }
  git_oid id;
  auto cid = ref->commitid(id);
  if (cid == nullptr) {
    return std::nullopt;
  }
  commit c;
  if (git_commit_lookup(&c.c, repo_, cid) != 0) {
    return std::nullopt;
  }
  return std::make_optional(std::move(c));
}

inline std::optional<commit>
repository::get_reference_commit_auto(std::string_view ref) {
  auto c = get_commit(ref);
  if (c) {
    return c;
  }
  if (aze::starts_with(ref, "refs/heads/") || ref == "HEAD") {
    return get_reference_commit(ref);
  }
  std::string xref("refs/heads/");
  xref.append(ref);
  return get_reference_commit(ref);
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
