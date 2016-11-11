/*
* analyze.cc
* git-analyze
* author: Force.Charlie
* Date: 2016.08
* Copyright (C) 2016. OSChina.NET. All Rights Reserved.
*/
#include <Pal.hpp>
#include <git2.h>
#include "analyze.hpp"
#include "analyze_internal.h"
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#ifdef _WIN32
#include <thread>
#include <Windows.h>

// VOID WINAPI OnTimerAPCProc(_In_opt_ LPVOID lpArgToCompletionRoutine,
//                            _In_ DWORD dwTimerLowValue,
//                            _In_ DWORD dwTimerHighValue) {
//   ////
//   fprintf(stderr, "git-analyze process timeout, exit !\n");
//   exit(-1);
// }

DWORD WINAPI AnalyzeTaskTimer(LPVOID lParam) {
  int t_ = *(reinterpret_cast<int *>(lParam));
  Sleep(t_ * 1000);
  BaseErrorMessagePrint("git-analyze process timeout, exit !\n");
  exit(-1);
  /// always noreturn
  return 0;
}

bool InitializeTaskTimer(int t_) {
  //
  // auto hTimer = CreateWaitableTimer(NULL, FALSE, NULL);
  // LARGE_INTEGER dueTime;
  // dueTime.QuadPart = t_ * -10000000;
  // if (hTimer == nullptr)
  //   return false;
  // if (!SetWaitableTimer(hTimer, &dueTime, 0, OnTimerAPCProc, NULL, FALSE)) {
  //   return false;
  // }
  DWORD tid;
  HANDLE hThread = CreateThread(NULL, 0, AnalyzeTaskTimer, &t_, 0, &tid);
  if (hThread == NULL) {
    BaseErrorMessagePrint("CreateThread() failed, LastErrorCode : %d",
                          GetLastError());
    return false;
  }
  CloseHandle(hThread);
  return true;
}

#else
#include <unistd.h>
#include <sys/signal.h>

void TimerSignalEvent(int sig) {
  (void)sig;
  BaseErrorMessagePrint("git-analyze process timeout, exit !\n");
  exit(-1);
}

bool InitializeTaskTimer(int t_) {
  signal(SIGALRM, TimerSignalEvent);
  alarm(static_cast<unsigned int>(t_));
  return true;
}
#endif

bool g_showcommitter = false;
std::int64_t g_limitsize = 100 * MBSIZE;
std::int64_t g_warnsize = 50 * MBSIZE;

void print_commit_message(const git_commit *commit_) {
  auto sig = git_commit_author(commit_);
  BaseConsoleWrite("author: %s <%s>\nmessage: %s\n\n", sig->name, sig->email,
                   git_commit_message(commit_));
}

int git_treewalk_resolveblobs(const char *root, const git_tree_entry *entry,
                              void *payload) {
  //
  if (git_tree_entry_type(entry) == GIT_OBJ_BLOB) {
    RaiiRepository *repo_ = reinterpret_cast<RaiiRepository *>(payload);
    git_object *obj{nullptr};
    if (git_tree_entry_to_object(&obj, repo_->repository(), entry) == 0) {
      auto blob = reinterpret_cast<git_blob *>(obj);
      auto size = git_blob_rawsize(blob);
      if (size >= g_warnsize) {
        auto cstr = git_oid_tostr_s(git_commit_id(repo_->commit()));
        BaseWarningMessagePrint("commit: %s file: %s%s (%4.2f MB)\n", cstr,
                                root, git_tree_entry_name(entry),
                                ((double)size / MBSIZE));
        if (g_showcommitter) {
          print_commit_message(repo_->commit());
        }
      }
    }
  }
  return 0;
}

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
    if (size > g_warnsize) {
      auto cstr = git_oid_tostr_s(git_commit_id(repo_->commit()));
      BaseWarningMessagePrint("commit: %s file: %s (%4.2f MB) \n", cstr,
                              delta->new_file.path, ((double)size / MBSIZE));
      if (g_showcommitter) {
        print_commit_message(repo_->commit());
      }
    }
  }
  return 0;
}

//// because git_reference_name_to_id require refs
bool RaiiRepository::refcommit(const char *refname) {
  git_reference *ref_{nullptr};
  if (git_reference_lookup(&ref_, repo_, refname) != 0) {
    //// second look branch to ref
    if (git_branch_lookup(&ref_, repo_, refname, GIT_BRANCH_LOCAL) != 0) {
      auto err = giterr_last();
      BaseErrorMessagePrint("Lookup reference and branch failed: %s\n",
                            err->message);
      return false;
    }
  }
  git_reference *dref_{nullptr};
  if (git_reference_resolve(&dref_, ref_) != 0) {
    git_reference_free(ref_);
    auto err = giterr_last();
    BaseErrorMessagePrint("Resolve reference failed: %s\n", err->message);
    return false;
  }
  //// we check branch, but branch ref type should GIT_REF_OID
  auto oid = git_reference_target(dref_);
  if (git_commit_lookup(&cur_commit_, repo_, oid) != 0) {
    git_reference_free(ref_);
    git_reference_free(dref_);
    auto err = giterr_last();
    BaseErrorMessagePrint("Lookup commit failed: %s\n", err->message);
    return false;
  }
  git_reference_free(ref_);
  git_reference_free(dref_);
  return true;
}

bool RaiiRepository::walk() {
  git_commit *parent = nullptr;
  git_tree *old_tree = nullptr;
  git_tree *new_tree = nullptr;
  //// Fix me when commit is first commit ,so, use git_tree_walk parse all blob
  if (git_commit_parent(&parent, cur_commit_, 0) != 0) {
    // auto err = giterr_last();
    if (git_commit_parentcount(cur_commit_) == 0) {
      if (git_commit_tree(&new_tree, cur_commit_) != 0) {
        //// cur_commit_ will released by self
        return false;
      }
      git_tree_walk(new_tree, GIT_TREEWALK_PRE, git_treewalk_resolveblobs,
                    this);
    }

    return false;
  }

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
    BaseErrorMessagePrint("git_branch_iterator_new: %s\n", err->message);
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
      BaseErrorMessagePrint("Lookup commit failed: %s\n", err->message);
      return false;
    }
    BaseConsoleWrite("git-analyze> reference: %s\n", git_reference_name(ref_));
    while (walk()) {
      /////
    }
    if (cur_commit_) {
      git_commit_free(cur_commit_);
      cur_commit_ = nullptr;
    }
    git_reference_free(ref_);
  }
  return true;
}

bool RaiiRepository::load(const char *dir) {
  if (git_repository_open(&repo_, dir) != 0) {
    auto err = giterr_last();
    BaseErrorMessagePrint("git-analyze error: %s\n", err->message);
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
      BaseErrorMessagePrint("create timer failed !\n");
    }
  }
  LibgitHelper helper;
  RaiiRepository repository;
  BaseConsoleWrite("git-analyze limit: %4.2f MB warning: %4.2f MB\n",
                   ((double)g_limitsize / MBSIZE),
                   ((double)g_warnsize / MBSIZE));
  if (!repository.load(analyzeArgs.repository.c_str())) {
    ////
    return false;
  }
  if (analyzeArgs.allrefs) {
    return repository.foreachref();
  } else {
    if (!repository.refcommit(analyzeArgs.ref.c_str()))
      return false;
    BaseConsoleWrite("git-analyze> ref (branch): %s\n",
                     analyzeArgs.ref.c_str());
    while (repository.walk()) {
      ///
    }
  }
  ////
  return true;
}
