/*
* rollback.cc
* git-rollback
* author: Force.Charlie
* Date: 2016.08
* Copyright (C) 2016. OSChina.NET. All Rights Reserved.
*/
#include <cstdio>
#include <cstring>
#include <string>
#include <git2.h>
#include "rollback.hpp"

RollbackDriver::RollbackDriver() { git_libgit2_init(); }

RollbackDriver::~RollbackDriver() { git_libgit2_shutdown(); }

//// Find
bool IsRelationshipCommit(git_repository *repo, git_reference *dref,
                          const git_oid *oid) {
  git_commit *commit_{nullptr};
  git_commit *parent_{nullptr};
  auto target_ = git_reference_target(dref);
  if (git_commit_lookup(&commit_, repo, target_) != 0) {
    return false;
  }
  for (; git_commit_parentcount(commit_) > 0;
       git_commit_parent(&parent_, commit_, 0)) {
    // git_commit_id(commit_)
    if (memcmp(git_commit_id(commit_), oid, sizeof(git_oid)) == 0) {
      git_commit_free(parent_);
      git_commit_free(commit_);
      return true;
    }
    git_commit_free(commit_);
    commit_ = parent_;
  }
  git_commit_free(commit_);
  return false;
}

bool RollbackWithRealCommit(git_reference *ref, const git_oid *id) {
  ///
  git_reference *newref_{nullptr};
  if (git_oid_cmp(id, git_reference_target(ref))) {
    printf("No rollback, reference %s commit is %s\n", git_reference_name(ref),
           git_oid_tostr_s(id));
    return true;
  }
  std::string log("rollback to old commit: ");
  log.append(git_oid_tostr_s(id));
  if (git_reference_set_target(&newref_, ref, id, log.c_str()) != 0) {
    auto er = giterr_last();
    fprintf(stderr, "rollback reference failed: %s\n", er->message);
    return false;
  }
  git_reference_free(newref_);
  return true;
}

bool RollbackDriver::RollbackWithCommit(const char *repodir,
                                        const char *refname, const char *hexid,
                                        bool forced) {
  /// to rollback with commit ,
  git_repository *repo_{nullptr};
  git_reference *ref_{nullptr};
  git_oid oid;
  if (git_oid_fromstr(&oid, hexid) != 0 || git_oid_iszero(&oid)) {
    fprintf(stderr, "Error Hexid %s\n", hexid);
    return false;
  }
  auto Release = [&]() {
    /// Release all
    if (ref_) {
      git_reference_free(ref_);
    }
    if (repo_) {
      git_repository_free(repo_);
    }
  };
  if (git_repository_open(&repo_, repodir) != 0) {
    auto err = giterr_last();
    fprintf(stderr, "%s\n", err->message);
    return false;
  }
  git_reference *xref;
  if (git_reference_lookup(&xref, repo_, refname) != 0) {
    if (git_branch_lookup(&xref, repo_, refname, GIT_BRANCH_LOCAL) != 0) {
      auto err = giterr_last();
      fprintf(stderr, "%s\n", err->message);
      Release();
      return false;
    }
  }
  if (git_reference_resolve(&ref_, xref) != 0) {
    git_reference_free(xref);
    Release();
    return false;
  }
  git_reference_free(xref);
  ///////////////////////////////////////////////
  if (!IsRelationshipCommit(repo_, ref_, &oid)) {
    Release();
    fprintf(stderr, "Not Found commit : %s In branch mainline\n", hexid);
    return false;
  }
  if (RollbackWithRealCommit(ref_, &oid)) {
    fprintf(stderr, "rollback ref: %s to commit: %s success\n",
            git_reference_name(ref_), hexid);
    Release();
    if (!GitGCInvoke(repodir, forced)) {
      fprintf(stderr, "Run GC failed\n");
      return false;
    }
    return true;
  }
  Release();
  return false;
}

bool RollbackDriver::RollbackWithRev(const char *repodir, const char *refname,
                                     int rev, bool forced) {
  if (rev < 0) {
    fprintf(stderr, "rollack version must large 0\n");
    return false;
  } else if (rev == 0) {
    printf("no rollback, rev=0 \n");
    return true;
  }
  ///////////////////////
  return true;
}
