/*
 * analyze.cc
 * git-analyze
 * author: Force.Charlie
 * Date: 2016.08
 * Copyright (C) 2019. GITEE.COM. All Rights Reserved.
 */
#include "analyze.hpp"
#include "analyze_internal.h"
#include <Pal.hpp>
#include <git2.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#ifdef _WIN32
#include <Windows.h>
#include <thread>

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
  Printe("git-analyze process timeout, exit !\n");
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
    Printe("CreateThread() failed, LastErrorCode : %d", GetLastError());
    return false;
  }
  CloseHandle(hThread);
  return true;
}

#else
#include <sys/signal.h>
#include <unistd.h>

void TimerSignalEvent(int sig) {
  (void)sig;
  Printe("git-analyze process timeout, exit !\n");
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
  Print("author: %s <%s>\nmessage: %s\n\n", sig->name, sig->email,
        git_commit_message(commit_));
}

int git_treewalk_resolveblobs(const char *root, const git_tree_entry *entry,
                              void *payload) {
  //
  if (git_tree_entry_type(entry) == GIT_OBJ_BLOB) {
    GitRepository *repo_ = reinterpret_cast<GitRepository *>(payload);
    git_object *obj{nullptr};
    if (git_tree_entry_to_object(&obj, repo_->repository(), entry) == 0) {
      auto blob = reinterpret_cast<git_blob *>(obj);
      auto size = git_blob_rawsize(blob);
      if (size >= g_warnsize) {
        auto cstr = git_oid_tostr_s(git_commit_id(repo_->commit()));
        Printw("\b\rcommit: %s file: %s%s (%4.2f MB)\n", cstr, root,
               git_tree_entry_name(entry), ((double)size / MBSIZE));
        if (g_showcommitter) {
          print_commit_message(repo_->commit());
        }
      }
      git_blob_free(blob);
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
    GitRepository *repo_ = static_cast<GitRepository *>(payload);
    const auto size = delta->new_file.size;
    if (size > g_warnsize) {
      auto cstr = git_oid_tostr_s(git_commit_id(repo_->commit()));
      Printw("\b\rcommit: %s file: %s (%4.2f MB) \n", cstr,
             delta->new_file.path, ((double)size / MBSIZE));
      if (g_showcommitter) {
        print_commit_message(repo_->commit());
      }
    }
  }
  return 0;
}

//// because git_reference_name_to_id require refs
bool GitRepository::refcommit(const char *refname) {
  git_reference *ref_{nullptr};
  if (git_reference_lookup(&ref_, repo_, refname) != 0) {
    //// second look branch to ref
    if (git_branch_lookup(&ref_, repo_, refname, GIT_BRANCH_LOCAL) != 0) {
      auto err = giterr_last();
      Printe("Lookup reference and branch failed: %s\n", err->message);
      return false;
    }
  }
  git_reference *dref_{nullptr};
  if (git_reference_resolve(&dref_, ref_) != 0) {
    git_reference_free(ref_);
    auto err = giterr_last();
    Printe("Resolve reference failed: %s\n", err->message);
    return false;
  }
  //// we check branch, but branch ref type should GIT_REF_OID
  auto oid = git_reference_target(dref_);
  if (git_commit_lookup(&cur_commit_, repo_, oid) != 0) {
    git_reference_free(ref_);
    git_reference_free(dref_);
    auto err = giterr_last();
    Printe("Lookup commit failed: %s\n", err->message);
    return false;
  }
  git_reference_free(ref_);
  git_reference_free(dref_);
  return true;
}

bool GitRepository::walk() {
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
      git_tree_free(new_tree);
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

bool GitRepository::foreachref() {
  git_branch_iterator *iter_{nullptr};
  if (git_branch_iterator_new(&iter_, repo_, GIT_BRANCH_LOCAL) != 0) {
    auto err = giterr_last();
    Printe("git_branch_iterator_new: %s\n", err->message);
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
      Printe("Lookup commit failed: %s\n", err->message);
      return false;
    }
    Print("git-analyze> reference: %s\n", git_reference_name(ref_));
    int i = 0;
    while (walk()) {
      i++;
      putc('\b', stdout);
      printf("\rcompleted: %d", i);
      fflush(stdout);
    }
    printf("\rresolve %s %d commits done\n", git_reference_name(ref_), i + 1);
    if (cur_commit_) {
      git_commit_free(cur_commit_);
      cur_commit_ = nullptr;
    }
    git_reference_free(ref_);
  }
  git_branch_iterator_free(iter_);
  return true;
}

bool GitRepository::load(const char *dir) {
  if (git_repository_open(&repo_, dir) != 0) {
    auto err = giterr_last();
    Printe("git-analyze error: %s\n", err->message);
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
      Printe("create timer failed !\n");
    }
  }
  LibgitHelper helper;
  GitRepository repository;
  Print("git-analyze limit: %4.2f MB warning: %4.2f MB\n",
        ((double)g_limitsize / MBSIZE), ((double)g_warnsize / MBSIZE));
  if (!repository.load(analyzeArgs.repository.c_str())) {
    ////
    return false;
  }
  if (analyzeArgs.allrefs) {
    return repository.foreachref();
  } else {
    if (!repository.refcommit(analyzeArgs.ref.c_str()))
      return false;
    Print("git-analyze> ref (branch): %s\n", analyzeArgs.ref.c_str());
    int i = 0;
    while (repository.walk()) {
      i++;
      putc('\b', stdout);
      printf("\rcompleted: %d", i);
      fflush(stdout);
    }
    printf(
        "\rresolve %d commits done (git rev-list --first-parent --count %s) \n",
        i + 1, analyzeArgs.ref.c_str());
  }
  ////
  return true;
}
