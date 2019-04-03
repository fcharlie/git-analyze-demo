///
#include <cstdio>
#include <cstdlib>
#include <finaly.hpp>
#include <console.hpp>
#include "executor.hpp"

namespace aze {

struct aze_diff {
  ~aze_diff() {
    if (diff != nullptr) {
      git_diff_free(diff);
    }
  }
  git_diff *diff{nullptr};
  bool resolve_diff(git_repository *repo_, git_tree *old_tree,
                    git_tree *new_tree) {
    git_diff_options opts;
    git_diff_init_options(&opts, GIT_DIFF_OPTIONS_VERSION); //
    if (git_diff_tree_to_tree(&diff, repo_, old_tree, new_tree, &opts) != 0) {
      return false;
    }
    return true;
  }
};

struct aze_payload_t {
  bool showcommitter{false};
  git_off_t wsize;
  git_repository *r{nullptr};
  git_commit *c;
};

constexpr const std::uint64_t MB = 1024 * 1024ull;

void print_commit_message(const git_commit *commit_) {
  auto sig = git_commit_author(commit_);
  aze::FPrintF(stderr, "author: %s <%s>\nmessage: %s\n\n", sig->name,
               sig->email, git_commit_message(commit_));
}

int git_treewalk_callback_impl(const char *root, const git_tree_entry *entry,
                               void *payload) {
  //
  if (git_tree_entry_type(entry) == GIT_OBJ_BLOB) {
    auto dv = reinterpret_cast<aze_payload_t *>(payload);
    git_object *obj{nullptr};
    if (git_tree_entry_to_object(&obj, dv->r, entry) == 0) {
      auto blob = reinterpret_cast<git_blob *>(obj);
      auto size = git_blob_rawsize(blob);
      if (size >= dv->wsize) {
        auto cstr = git_oid_tostr_s(git_commit_id(dv->c));
        aze::FPrintF(stderr, "\rcommit: %s file: %s%s (%4.2f MB)\n", cstr, root,
                     git_tree_entry_name(entry), ((double)size / MB));
        if (dv->showcommitter) {
          print_commit_message(dv->c);
        }
      }
      git_blob_free(blob);
    }
  }
  return 0;
}

int git_diff_callback(const git_diff_delta *delta, float progress,
                      void *payload) {
  (void)progress;
  if (delta->status == GIT_DELTA_ADDED || delta->status == GIT_DELTA_MODIFIED) {
    /* code */
    // GitRepository *repo_ = static_cast<GitRepository *>(payload);
    auto dv = reinterpret_cast<aze_payload_t *>(payload);
    if (delta->new_file.size > (git_off_t)dv->wsize) {
      auto cstr = git_oid_tostr_s(git_commit_id(dv->c));
      aze::FPrintF(stderr, "\rcommit: %s file: %s (%4.2f MB) \n", cstr,
                   delta->new_file.path, ((double)delta->new_file.size / MB));
      if (dv->showcommitter) {
        print_commit_message(dv->c);
      }
    }
  }
  return 0;
}

bool Executor::AzeOneInternal(const git_oid *id) {
  aze_payload_t dv;
  dv.r = r.p();
  dv.showcommitter = showcommitter;
  dv.wsize = ws;
  git_commit *cur = nullptr;
  git_commit *parent = nullptr;
  git_tree *old_tree = nullptr;
  git_tree *new_tree = nullptr;
  auto fa = aze::finally([&] {
    if (cur != nullptr) {
      git_commit_free(cur);
    }
    if (parent != nullptr) {
      git_commit_free(parent);
    }
    if (old_tree != nullptr) {
      git_tree_free(old_tree);
    }
    if (new_tree != nullptr) {
      git_tree_free(new_tree);
    }
  });

  if (git_commit_lookup(&cur, r.p(), id) != 0) {
    return false;
  }
  if (git_commit_tree(&new_tree, cur) != 0) {
    return false;
  }
  int i = 0;
  while (git_commit_parentcount(cur) > 0) {
    if (git_commit_parent(&parent, cur, 0) != 0) {
      return false;
    }
    if (git_commit_tree(&old_tree, parent) != 0) {
      return false;
    }
    aze_diff diff;
    if (!diff.resolve_diff(r.p(), old_tree, new_tree)) {
      return false;
    }
    dv.c = cur;
    git_diff_foreach(diff.diff, git_diff_callback, nullptr, nullptr, nullptr,
                     &dv);

    git_tree_free(new_tree);
    new_tree = old_tree;
    old_tree = nullptr;
    git_commit_free(cur);
    cur = parent;
    parent = nullptr;
    i++;
    aze::FPrintF(stderr, "\rcompleted: %d", i);
  }
  dv.c = cur;
  git_tree_walk(new_tree, GIT_TREEWALK_PRE, git_treewalk_callback_impl, &dv);
  i++;
  aze::FPrintF(stderr, "\rcompleted: %d\n", i);
  return true;
}

bool Executor::AzeOne(std::string_view refname, std::int64_t timeout) {
  if (timeout > 0) {
    t.async_wait(timeout, [] {
      aze::FPrintF(stderr, "git-analyze process timeout, exit !\n");
      exit(1);
    });
  }
  auto c = r.get_reference_commit_auto(refname);
  if (!c) {
    return false;
  }
  if (AzeOneInternal(git_commit_id(c->p()))) {
    aze::FPrintF(stderr, "Branch '%s' resolve done\n", refname.data());
    return true;
  }
  return false;
}

bool Executor::AzeAll(std::int64_t timeout) {
  if (timeout > 0) {
    t.async_wait(timeout, [] {
      aze::FPrintF(stderr, "git-analyze process timeout, exit !\n");
      exit(1);
    });
  }

  git_branch_iterator *iter_{nullptr};
  if (git_branch_iterator_new(&iter_, r.p(), GIT_BRANCH_LOCAL) != 0) {
    auto err = giterr_last();
    aze::FPrintF(stderr, "git_branch_iterator_new: %s\n", err->message);
    return false;
  }
  git_reference *ref_{nullptr};
  git_branch_t tb;
  /// bacause we set git_branch_iterator_new GIT_BRANCH_LOCAL.
  while (git_branch_next(&ref_, &tb, iter_) == 0) {
    auto oid = git_reference_target(ref_);
    if (AzeOneInternal(oid)) {
      aze::FPrintF(stderr, "\rresolve %s done\n", git_reference_name(ref_));
    }
    git_reference_free(ref_);
  }
  git_branch_iterator_free(iter_);
  return true;
}

bool Executor::Initialize(std::string_view gitdir) {
  git::error_code ec;
  auto o = git::repository::make_repository_ex(gitdir, ec);
  if (!o) {
    aze::FPrintF(stderr, "unable open repository: %s\n", ec.message);
    return false;
  }
  r.acquire(std::move(*o));
  return true;
}

} // namespace aze
