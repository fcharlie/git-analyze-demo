////////
#ifndef AZE_IMPL_GIT_IPP
#define AZE_IMPL_GIT_IPP
#include <absl/strings/str_cat.h>
#include "git.hpp"

namespace git {

inline bool commit::equal(const git_oid *id) {
  auto xid = git_commit_id(c);
  if (xid == nullptr) {
    return false;
  }
  return git_oid_cmp(id, xid) == 0;
}

inline std::vector<commit> commit::parents() {
  std::vector<commit> cv;
  auto n = git_commit_parentcount(c);
  for (unsigned int i = 0; i < n; i++) {
    commit pc;
    if (git_commit_parent(&pc.c, c, i) != 0) {
      return cv;
    }
    cv.push_back(std::move(pc));
  }
  return cv;
}

inline std::optional<tree> tree::get_tree(repository &r, commit &c,
                                          std::string_view path) {
  tree t;
  if (git_commit_tree(&t.tree_, c.p()) != 0) {
    return std::nullopt;
  }
  if (path.empty() || path == ".") {
    return std::make_optional(std::move(t));
  }
  git_tree_entry *te = nullptr;
  if (git_tree_entry_bypath(&te, t.p(), path.data()) != 0) {
    return std::nullopt;
  }
  if (git_tree_entry_type(te) != GIT_OBJ_TREE) {
    git_tree_entry_free(te);
    return std::nullopt;
  }
  tree ct;
  if (git_tree_lookup(&ct.tree_, r.p(), git_tree_entry_id(te)) != 0) {
    git_tree_entry_free(te);
    return std::nullopt;
  }
  git_tree_entry_free(te);
  return std::make_optional(std::move(ct));
}

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
  if (git_reference_lookup(&r.ref_, repo_, refname.data()) != 0) {
    return std::nullopt;
  }
  if (git_reference_type(r.p()) == GIT_REFERENCE_SYMBOLIC) {
    reference rd;
    if (git_reference_resolve(&rd.ref_, r.p()) != 0) {
      return std::nullopt;
    }
    return std::make_optional(std::move(rd));
  }
  return std::make_optional(std::move(r));
}

inline std::optional<reference>
repository::get_branch(std::string_view branch) {
  reference r;
  if (git_branch_lookup(&r.ref_, repo_, branch.data(), GIT_BRANCH_LOCAL) == 0) {
    return std::make_optional(std::move(r));
  }
  return std::nullopt;
}

// When only resolve one peel object. tag exists.
inline std::optional<commit> repository::get_commit(const git_oid *id) {
  git_object *obj = nullptr;
  if (git_object_lookup(&obj, repo_, id, GIT_OBJECT_ANY) != 0) {
    return std::nullopt;
  }
  switch (git_object_type(obj)) {
  case GIT_OBJECT_COMMIT: {
    commit c;
    c.c = reinterpret_cast<git_commit *>(obj);
    return std::make_optional(std::move(c));
  }
  case GIT_OBJECT_TAG:
    break;
  default:
    git_object_free(obj);
    return std::nullopt;
  }
  git_object *peel = nullptr;
  if (git_object_peel(&peel, obj, GIT_OBJECT_COMMIT) != 0) {
    /* code */
    git_object_free(obj);
    return std::nullopt;
  }
  git_object_free(obj);
  commit c;
  c.c = reinterpret_cast<git_commit *>(peel);
  return std::make_optional(std::move(c));
}

inline std::optional<commit> repository::get_commit(std::string_view oid) {
  git_oid id;
  if (git_oid_fromstrn(&id, oid.data(), oid.size()) != 0) {
    return std::nullopt;
  }
  return get_commit(&id);
}

inline std::optional<commit>
repository::get_reference_commit(std::string_view refname) {
  auto ref = get_reference(refname);
  if (!ref) {
    return std::nullopt;
  }
  git_oid id;
  auto oid = ref->commitid(id);
  if (oid == nullptr) {
    return std::nullopt;
  }
  return get_commit(oid);
}

inline std::optional<commit>
repository::get_reference_commit_auto(std::string_view ref) {
  auto c = get_commit(ref);
  if (c) {
    return c;
  }
  if (absl::StartsWith(ref, "refs/heads/") || ref == "HEAD") {
    return get_reference_commit(ref);
  }
  auto xref = absl::StrCat("refs/heads/", ref);
  return get_reference_commit(xref);
}

inline std::optional<config> repository::get_config() {
  auto p = absl::StrCat(git_repository_path(repo_), "/config");
  config c;
  if (git_config_open_ondisk(&c.c, p.c_str()) != 0) {
    return std::nullopt;
  }
  return std::make_optional(std::move(c));
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

inline std::optional<repository>
repository::make_repository_ex(std::string_view sv, error_code &ec) {
  repository repo;
  ec.ec = git_repository_open_ext(&repo.repo_, sv.data(), 0, nullptr);
  if (ec.ec != 0) {
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
