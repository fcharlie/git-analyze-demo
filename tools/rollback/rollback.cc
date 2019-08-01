/*
 * rollback.cc
 * git-rollback
 * author: Force.Charlie
 * Date: 2016.08
 * Copyright (C) 2019. GITEE.COM. All Rights Reserved.
 */
#include <cstdio>
#include <cstring>
#include <string>
#include <git2.h>
#include <console.hpp>
#include <git.hpp>
#include "rollback.hpp"

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
  bool initialize(git_repository *r) {
    return (git_revwalk_new(&walk, r) == 0);
  }
  bool initialize_ref(git_repository *r, git_reference *ref) {
    if (git_revwalk_new(&walk, r) != 0) {
      return false;
    }
    return (git_revwalk_push_ref(walk, git_reference_name(ref)) == 0);
  }

  git_revwalk *walk{nullptr};
};

bool RevExists(git::repository &r, git::reference &ref, const git_oid *id) {
  revwalk_t w;
  if (!w.initialize_ref(r.p(), ref.p())) {
    return false;
  }
  git_oid oid;
  while (git_revwalk_next(&oid, w.walk) == 0) {
    if (git_oid_cmp(id, &oid) == 0) {
      return true;
    }
  }
  return false;
}

bool ApplyNewOID(git::repository &r, std::string_view refname,
                 std::string_view oid) {
  git_oid id;
  if (git_oid_fromstrn(&id, oid.data(), oid.size()) != 0) {
    return false;
  }
  auto ref = r.get_reference(refname);
  if (!ref) {
    auto e = giterr_last();
    aze::FPrintF(stderr, "unable open refname '%s' error: %s\n", refname,
                 e->message);
    return false;
  }
  if (!RevExists(r, *ref, &id)) {
    aze::FPrintF(stderr, "commit %s not exists in %s history\n", oid, refname);
    return false;
  }
  auto msg = absl::StrCat("rollback '", refname, "' to commit id'", oid, "'");
  auto nr = ref->new_target(&id, msg);
  if (!nr) {
    auto e = giterr_last();
    aze::FPrintF(stderr, "unable set '%s' target to %s error: %s\n", refname,
                 oid, e->message);
    return false;
  }
  return true;
}

bool RevExists(git::repository &r, git::reference &ref, int rev, git_oid *id) {
  revwalk_t w;
  if (!w.initialize_ref(r.p(), ref.p())) {
    return false;
  }
  git_revwalk_simplify_first_parent(w.walk);
  git_oid oid;
  int i = 0;
  while (git_revwalk_next(&oid, w.walk) == 0) {
    if (i == rev) {
      git_oid_cpy(id, &oid);
      return true;
    }
    i++;
  }
  return false;
}

bool ApplyBackRev(git::repository &r, std::string_view refname, int rev) {
  auto ref = r.get_reference(refname);
  if (!ref) {
    auto e = giterr_last();
    aze::FPrintF(stderr, "unable open refname '%s' error: %s\n", refname,
                 e->message);
    return false;
  }
  git_oid rid;
  if (!RevExists(r, *ref, rev, &rid)) {
    aze::FPrintF(stderr, "rev %d overflow\n", rev);
    return false;
  }
  auto msg = absl::StrCat("rollback '", refname, "' to commit id'",
                          git_oid_tostr_s(&rid), "'");
  auto nr = ref->new_target(&rid, msg);
  if (!nr) {
    auto e = giterr_last();
    aze::FPrintF(stderr, "unable set '%s' target to %s error: %s\n", refname,
                 git_oid_tostr_s(&rid), e->message);
    return false;
  }
  return true;
}

bool ExecuteWithOptions(const rollback_options &opt) {
  git::error_code ec;
  auto r = git::repository::make_repository_ex(opt.gitdir, ec);
  if (!r) {
    aze::FPrintF(stderr, "unable open '%s' error: %s\n", opt.gitdir,
                 ec.message);
    return false;
  }
  if (!opt.oid.empty()) {
    return ApplyNewOID(*r, opt.refname, opt.oid);
  }
  return ApplyBackRev(*r, opt.refname, opt.rev);
}
