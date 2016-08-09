/*
* analyze.cc
* git-analyze
* author: Force.Charlie
* Date: 2016.08
* Copyright (C) 2016. OSChina.NET. All Rights Reserved.
*/
#include <git2.h>
#include "analyze.hpp"
#include "analyze_internal.h"
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#ifdef _WIN32
#include <Windows.h>

VOID WINAPI OnTimerAPCProc(_In_opt_ LPVOID lpArgToCompletionRoutine,
                           _In_ DWORD dwTimerLowValue,
                           _In_ DWORD dwTimerHighValue) {
  ////
  fprintf(stderr, "git-analyze process timeout, exit !\n");
  exit(-1);
}

bool InitializeTaskTimer(std::int64_t t_) {
  //
  auto hTimer = CreateWaitableTimer(NULL, FALSE, NULL);
  LARGE_INTEGER dueTime;
  dueTime.QuadPart = t_ * -10000000;
  if (hTimer == nullptr)
    return false;
  if (!SetWaitableTimer(hTimer, &dueTime, 0, OnTimerAPCProc, NULL, FALSE)) {
    return false;
  }
  return true;
}

#else
#include <unistd.h>
#include <sys/signal.h>

void TimerSignalEvent(int sig) {
  (void)sig;
  fprintf(stderr, "git-analyze process timeout, exit !\n");
  exit(-1);
}

bool InitializeTaskTimer(std::int64_t t_) {
  signal(SIGALRM, TimerSignalEvent);
  alarm(static_cast<unsigned int>(t_));
  return true;
}
#endif

std::int64_t g_limitsize = 100 * MBSIZE;
std::int64_t g_warnsize = 50 * MBSIZE;

//// callback
int git_diff_callback(const git_diff_delta *delta, float progress,
                      void *payload) {
  (void)progress;
  if (delta->status == GIT_DELTA_ADDED || delta->status == GIT_DELTA_MODIFIED) {
    /* code */
    RaiiRepository *repo_ = static_cast<RaiiRepository *>(payload);
    git_blob *blob = nullptr;
    if (git_blob_lookup(&blob, repo_->repository(), &(delta->new_file.id)) !=
        0) {
      return 0;
    }
    //// by default off_t is 8byte
    git_off_t size = git_blob_rawsize(blob);
    if (size > g_limitsize) {
      auto cstr = git_oid_tostr_s(git_commit_id(repo_->commit()));
      fprintf(stderr, "commit: %s file: %s size<limit>: %4.2f MB\n", cstr,
              delta->new_file.path, ((double)size / MBSIZE));
    } else if (size > g_warnsize) {
      auto cstr = git_oid_tostr_s(git_commit_id(repo_->commit()));
      fprintf(stderr, "commit: %s file: %s size<warning>: %4.2f MB\n", cstr,
              delta->new_file.path, ((double)size / MBSIZE));
    }
  }
  return 0;
}

bool RaiiRepository::refcommit(const char *refname) {
  git_reference *ref_{nullptr};
  if (git_reference_lookup(&ref_, repo_, refname) != 0) {
    //// second look branch to ref
    if (git_branch_lookup(&ref_, repo_, refname, GIT_BRANCH_LOCAL) != 0) {
      auto err = giterr_last();
      fprintf(stderr, "Parse ref failed: %s\n", err->message);
      return false;
    }
  }
  git_reference *dref_{nullptr};
  if (git_reference_resolve(&dref_, ref_) != 0) {
    git_reference_free(ref_);
    auto err = giterr_last();
    fprintf(stderr, "Resolve ref failed: %s\n", err->message);
    return false;
  }
  auto oid = git_reference_target(dref_);
  if (git_commit_lookup(&cur_commit_, repo_, oid) != 0) {
    git_reference_free(ref_);
    git_reference_free(dref_);
    auto err = giterr_last();
    fprintf(stderr, "Lookup commit: %s\n", err->message);
    return false;
  }
  git_reference_free(ref_);
  git_reference_free(dref_);
  return true;
}

bool RaiiRepository::walk() {
  git_commit *parent = nullptr;
  if (git_commit_parent(&parent, cur_commit_, 0) != 0) {
    return false;
  }
  git_tree *old_tree = nullptr;
  git_tree *new_tree = nullptr;
  if (git_commit_tree(&old_tree, parent) != 0) {
    git_commit_free(parent);
    return false;
  }
  if (git_commit_tree(&new_tree, cur_commit_) != 0) {
    git_tree_free(old_tree);
    git_commit_free(parent);
    return false;
  }
  git_diff_options opts = GIT_DIFF_OPTIONS_INIT;
  git_diff *diff = nullptr; ////
  if (git_diff_tree_to_tree(&diff, repo_, old_tree, new_tree, &opts) != 0) {
    git_tree_free(old_tree);
    git_tree_free(new_tree);
    git_commit_free(parent);
    return false;
  }
  git_diff_foreach(diff, git_diff_callback, NULL, NULL, NULL, this);
  git_diff_free(diff);
  git_tree_free(old_tree);
  git_tree_free(new_tree);
  git_commit_free(cur_commit_);
  cur_commit_ = parent;
  return true;
}

bool RaiiRepository::foreachref() {
  git_branch_iterator *iter_{nullptr};
  if (git_branch_iterator_new(&iter_, repo_, GIT_BRANCH_LOCAL) != 0) {
    auto err = giterr_last();
    fprintf(stderr, "git_branch_iterator_new: %s\n", err->message);
    return false;
  }
  git_reference *ref_{nullptr};
  git_branch_t tb;
  /// bacause we set git_branch_iterator_new GIT_BRANCH_LOCAL.
  while (git_branch_next(&ref_, &tb, iter_) == 0) {
    auto oid = git_reference_target(ref_);
    if (git_commit_lookup(&cur_commit_, repo_, oid) != 0) {
      git_reference_free(ref_);
      auto err = giterr_last();
      fprintf(stderr, "Parse reference: %s\n", err->message);
      return false;
    }
    printf("Parse ref: %s\n", git_reference_name(ref_));
    while (walk()) {
      /////
    }
    git_commit_free(cur_commit_);
    cur_commit_ = nullptr;
    git_reference_free(ref_);
  }
  return true;
}

bool RaiiRepository::load(const char *dir) {
  if (git_repository_open(&repo_, dir) != 0) {
    auto err = giterr_last();
    fprintf(stderr, "ERROR: %s\n", err->message);
    return false;
  }
  return true;
}

class LibgitHelper {
public:
  LibgitHelper() { git_libgit2_init(); }
  ~LibgitHelper() { git_libgit2_shutdown(); }
};

bool ProcessAnalyzeTask(const AnalyzeArgs &analyzeArgs) {
  if (analyzeArgs.timeout != -1) {
    if (!InitializeTaskTimer(analyzeArgs.timeout)) {
      fprintf(stderr, "create timer failed !\n");
    }
  }
  LibgitHelper helper;
  RaiiRepository repository;
  if (!repository.load(analyzeArgs.repository.c_str())) {
    ////
    return false;
  }
  if (analyzeArgs.allrefs) {
    return repository.foreachref();
  } else {
    if (!repository.refcommit(analyzeArgs.ref.c_str()))
      return false;
    printf("Parse single ref: %s\n", analyzeArgs.ref.c_str());
    while (repository.walk()) {
      ///
    }
  }
  ////
  return true;
}
