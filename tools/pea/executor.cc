// private email checker
#include <absl/strings/str_split.h>
#include <git.hpp>
#include "executor.hpp"

// check commit emails exists in private-email set.
namespace aze {
bool Executor::Initialize() {
  auto ex = std::getenv("GITEE_PEA");
  if (ex == nullptr) {
    // unset PRIVATE EMAIL skip
    return false;
  }
  std::vector<absl::string_view> ev =
      absl::StrSplit(ex, absl::ByChar(';'), absl::SkipEmpty()); // skip empty
  for (auto c : ev) {
    emails.insert(std::string(c));
  }
  return true;
}

constexpr const std::string_view zid =
    "0000000000000000000000000000000000000000";

struct revwalk_t {
  revwalk_t() = default;
  revwalk_t(const revwalk_t &) = delete;
  revwalk_t &operator=(const revwalk_t &) = delete;
  revwalk_t(revwalk_t &&other) {
    if (walk != nullptr) {
      git_revwalk_free(walk);
    }
    walk = other.walk;
    other.walk = nullptr;
  }
  ~revwalk_t() {
    if (walk != nullptr) {
      git_revwalk_free(walk);
    }
  }
  bool initialize_range(git_repository *r, absl::string_view oldrev,
                        absl::string_view newrev) {
    if (git_revwalk_new(&walk, r) != 0) {
      return false;
    }
    if (oldrev != zid) {
      git_oid id;
      if (git_oid_fromstrn(&id, oldrev.data(), oldrev.size()) != 0) {
        return false;
      }
      if (git_revwalk_hide(walk, &id) != 0) {
        return false;
      }
    }
    git_oid id;
    if (git_oid_fromstrn(&id, newrev.data(), newrev.size()) != 0) {
      return false;
    }
    return git_revwalk_push(walk, &id) == 0;
  }
  git_revwalk *walk{nullptr};
};

/*
Situation 1:
 newrev == zeroid
 delete refs skip.

Situation 2:
 oldrev == zeroid
 check all parents

Situation 3:
 ---> A ---> B
 oldrev A newrev B
 B Full parents.

Situation 4:
 ---> A ---> B
 oldrev B newrev A
 Skip ?

Situation 5:
 ---> A ---> B
 ---> A ---> C
 oldrev B newrev C
 C --> A
*/

bool Executor::Execute(std::string_view gitdir, std::string_view oldrev,
                       std::string_view newrev) {

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
  revwalk_t w;
  if (!w.initialize_range(r->p(), oldrev, newrev)) {
    return false;
  }
  git_oid oid;
  while (git_revwalk_next(&oid, w.walk) == 0) {
    auto c = r->get_commit(&oid);
    if (!c) {
      break;
    }
    auto author = git_commit_author(c->p());
    if (Exists(author->email)) {
      blocked = true;
      return false;
    }
    auto commiter = git_commit_committer(c->p());
    if (Exists(commiter->email)) {
      blocked = true;
      return false;
    }
  }

  return true;
}
} // namespace aze
