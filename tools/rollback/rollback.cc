/*
* rollback.cc
* git-rollback
* author: Force.Charlie
* Date: 2016.08
* Copyright (C) 2017. OSChina.NET. All Rights Reserved.
*/
#include <cstdio>
#include <cstring>
#include <string>
#include <git2.h>
#include <Pal.hpp>
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
  do {
    if (memcmp(git_commit_id(commit_), oid, sizeof(git_oid)) == 0) {
      git_commit_free(parent_);
      git_commit_free(commit_);
      return true;
    }
    if (git_commit_parent(&parent_, commit_, 0) != 0) {
      break;
    }
    git_commit_free(commit_);
    commit_ = parent_;
  } while (true);
  git_commit_free(commit_);
  return false;
}

bool RollbackWithRealCommit(git_reference *ref, const git_oid *id) {
  ///
  git_reference *newref_{nullptr};
  if (git_oid_cmp(id, git_reference_target(ref)) == 0) {
    BaseConsoleWrite("Rollback aborted, reference %s commit is %s\n",
                     git_reference_name(ref), git_oid_tostr_s(id));
    return false;
  }
  std::string log("rollback to old commit: ");
  log.append(git_oid_tostr_s(id));
  if (git_reference_set_target(&newref_, ref, id, log.c_str()) != 0) {
    auto er = giterr_last();
    BaseErrorMessagePrint("rollback reference failed: %s\n", er->message);
    return false;
  }
  git_reference_free(newref_);
  return true;
}

bool RollbackWithRealRevision(git_repository *repo, git_reference *dref,
                              int rev) {
  git_commit *commit_{nullptr};
  git_commit *parent_{nullptr};
  auto target_ = git_reference_target(dref);
  if (git_commit_lookup(&commit_, repo, target_) != 0) {
    return false;
  }
  int i = 0;
  do {
    if (i == rev) {
      auto id = git_commit_id(commit_);
      auto result = RollbackWithRealCommit(dref, id);
      git_commit_free(commit_);
      return result;
    }
    if (git_commit_parent(&parent_, commit_, 0) != 0) {
      break;
    }
    i++;
    git_commit_free(commit_);
    commit_ = parent_;
  } while (true);
  BaseErrorMessagePrint("git-rollback: Over commit, rollback broken !\n");
  git_commit_free(commit_);
  return false;
}

bool RollbackDriver::RollbackWithCommit(const char *repodir,
                                        const char *refname, const char *hexid,
                                        bool forced) {
  /// to rollback with commit ,
  git_repository *repo_{nullptr};
  git_reference *ref_{nullptr};
  git_oid oid;
  if (git_oid_fromstr(&oid, hexid) != 0 || git_oid_iszero(&oid)) {
    BaseErrorMessagePrint("Error Hexid %s\n", hexid);
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
    BaseErrorMessagePrint("%s\n", err->message);
    return false;
  }
  git_reference *xref;
  if (git_reference_lookup(&xref, repo_, refname) != 0) {
    if (git_branch_lookup(&xref, repo_, refname, GIT_BRANCH_LOCAL) != 0) {
      auto err = giterr_last();
      BaseErrorMessagePrint("%s\n", err->message);
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
    BaseErrorMessagePrint("Not Found commit : %s In branch mainline\n", hexid);
    return false;
  }
  auto result = RollbackWithRealCommit(ref_, &oid);
  if (result) {
    BaseConsoleWrite("rollback ref: %s to commit: %s success\n",
                     git_reference_name(ref_), hexid);
    Release();
    if (GitGCInvoke(repodir, forced)) {
      Release();
      return true;
    }
    BaseErrorMessagePrint("git-rollback: run git gc failed !\n");
  }
  Release();
  return false;
}

bool RollbackDriver::RollbackWithRev(const char *repodir, const char *refname,
                                     int rev, bool forced) {
  if (rev < 0) {
    BaseErrorMessagePrint("git-rollback: rollack revision rev must >0\n");
    return false;
  } else if (rev == 0) {
    BaseConsoleWrite("no rollback, rev=0 \n");
    return true;
  }
  git_repository *repo_{nullptr};
  git_reference *ref_{nullptr};

  auto Release = [&]() {
    if (ref_) {
      git_reference_free(ref_);
    }
    if (repo_) {
      git_repository_free(repo_);
    }
  };

  if (git_repository_open(&repo_, repodir) != 0) {
    auto err = giterr_last();
    BaseErrorMessagePrint("git-rollback error: %s\n", err->message);
    return false;
  }
  git_reference *xref;
  if (git_reference_lookup(&xref, repo_, refname) != 0) {
    if (git_branch_lookup(&xref, repo_, refname, GIT_BRANCH_LOCAL) != 0) {
      auto err = giterr_last();
      BaseErrorMessagePrint("lookup reference: %s\n", err->message);
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

  if (RollbackWithRealRevision(repo_, ref_, rev)) {
    BaseConsoleWrite("git-rollback: rollback success !\n");
    if (GitGCInvoke(repodir, forced)) {
      Release();
      return true;
    }
    BaseErrorMessagePrint("git-rollback: run git gc failed !\n");
  }
  ///////////////////////
  Release();
  return false;
}
