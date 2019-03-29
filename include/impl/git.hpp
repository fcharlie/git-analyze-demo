///
#ifndef AZE_GIT_IMPL_HPP
#define AZE_GIT_IMPL_HPP
#include <string>
#include <vector>
#include <optional>
#include "../string.hpp"
#include <git2.h>

namespace git {
struct error_code {
  std::string message;
  int ec;
  explicit operator bool() const noexcept { return ec == 0; }
};

// libgit2 initialize helper
class global_initializer_t {
public:
  global_initializer_t() { git_libgit2_init(); }
  ~global_initializer_t() { git_libgit2_shutdown(); }
  global_initializer_t(const global_initializer_t &) = delete;
  global_initializer_t &operator=(const global_initializer_t &) = delete;

private:
};

class repository;
class reference;
class commit;
class treeex;

class tree {
public:
  tree() = default;
  tree(tree &&other) {
    if (tree_ != nullptr) {
      git_tree_free(tree_);
    }
    tree_ = other.tree_;
    other.tree_ = nullptr;
  }
  tree(const tree &) = delete;
  tree &operator=(const tree &) = delete;
  ~tree() {
    if (tree_ != nullptr) {
      git_tree_free(tree_);
    }
  }
  static std::optional<tree> get_tree(repository &r, commit &c,
                                      std::string_view path);
  git_tree *p() { return tree_; }

private:
  git_tree *tree_{nullptr};
};

class commit {
public:
  commit() = default;
  commit(const commit &) = delete;
  commit &operator=(const commit &) = delete;
  commit(commit &&other) {
    if (c) {
      git_commit_free(c);
    }
    c = other.c;
    other.c = nullptr;
  }
  ~commit() {
    if (c != nullptr) {
      git_commit_free(c);
    }
  }
  git_commit *p() const { return c; }
  std::vector<commit> parents() {
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
  bool equal(const git_oid *id) {
    auto xid = git_commit_id(c);
    if (xid == nullptr) {
      return false;
    }
    return git_oid_cmp(id, xid) == 0;
  }

private:
  friend class repository;
  friend class reference;
  git_commit *c{nullptr};
};

class reference {
public:
  reference() = default;
  reference(reference &&other) {
    if (ref_ != nullptr) {
      git_reference_free(ref_);
    }
    ref_ = other.ref_;
    other.ref_ = nullptr;
  }
  reference(const reference &) = delete;
  reference &operator=(const reference &) = delete;
  ~reference() {
    if (ref_ != nullptr) {
      git_reference_free(ref_);
    }
  }
  std::optional<reference> symbolic_target() {
    if (git_reference_type(ref_) != GIT_REFERENCE_SYMBOLIC) {
      return std::nullopt;
    }
    reference dr;
    if (git_reference_resolve(&dr.ref_, ref_) != 0) {
      return std::nullopt;
    }
    return std::make_optional(std::move(dr));
  }
  const git_oid *commitid(git_oid &id) {
    switch (git_reference_type(ref_)) {
    case GIT_REFERENCE_DIRECT:
      return git_reference_target(ref_);
    case GIT_REFERENCE_SYMBOLIC: {
      git_reference *dr = nullptr;
      if (git_reference_resolve(&dr, ref_) != 0) {
        return nullptr;
      }
      auto p = git_reference_target(dr);
      git_oid_cpy(&id, p);
      git_reference_free(dr);
      return &id;
    }
    default:
      break;
    }
    return nullptr;
  }
  git_reference *p() const { return ref_; }

private:
  friend class repository;
  git_reference *ref_{nullptr};
};

// repository helper
class repository {
public:
  repository() = default;
  repository(repository &&other);
  repository(const repository &) = delete;
  repository &operator=(const repository &) = delete;
  ~repository() {
    if (repo_ != nullptr) {
      git_repository_free(repo_);
    }
  }

  git_repository *p() { return repo_; }
  std::optional<reference> get_reference(std::string_view refname);
  std::optional<reference> get_branch(std::string_view branch);
  std::optional<commit> get_reference_commit(std::string_view ref);
  std::optional<commit> get_reference_commit_auto(std::string_view ref);
  std::optional<commit> get_commit(std::string_view oid);
  static std::optional<repository> make_repository(std::string_view sv,
                                                   error_code &ec);

private:
  ::git_repository *repo_{nullptr};
};

} // namespace git

#endif
