/////
#include <git.hpp>
#include <console.hpp>
#include "cheat.hpp"

bool duplicate_new_branch(git_repository *repo, git_commit *commit,
                          git_tree *tree, const std::string &branch) {
  git_oid newoid;
  std::string refname = std::string("refs/heads/") + branch;
  auto au = git_commit_author(commit);
  auto cu = git_commit_committer(commit);
  auto msg = git_commit_message(commit);
  if (git_commit_create(&newoid, repo, refname.c_str(), au, cu, nullptr, msg,
                        tree, 0, nullptr) != 0) {
    auto e = giterr_last();
    aze::FPrintF(stderr, "create new branch %s failed: %s\n", branch,
                 e->message);
    return false;
  }
  aze::FPrintF(stderr, "[%s %s]\ncommitter: %s\nemail: %s\nmessage: %s\n\n",
               branch, git_oid_tostr_s(&newoid), cu->name, cu->email, msg);
  return true;
}

bool hack_new_branch(git_repository *repo, git_tree *tree,
                     const cheat_options &opt) {
  git_signature *au;
  git_signature *cu;
  int result = 0;
  if (!opt.author.empty() && !opt.authoremail.empty()) {
    if (opt.date == 0) {
      result =
          git_signature_now(&au, opt.author.c_str(), opt.authoremail.c_str());
    } else {
      result =
          git_signature_new(&au, opt.author.c_str(), opt.authoremail.c_str(),
                            opt.date, opt.timeoff);
    }
  } else {
    result = git_signature_default(&au, repo);
  }
  if (result != 0) {
    return false;
  }
  if (!opt.committer.empty() && !opt.commiteremail.empty()) {
    if (opt.date == 0) {
      result =
          git_signature_now(&cu, opt.author.c_str(), opt.authoremail.c_str());
    } else {
      result =
          git_signature_new(&cu, opt.author.c_str(), opt.authoremail.c_str(),
                            opt.date, opt.timeoff);
    }
  } else {
    result = git_signature_default(&cu, repo);
  }
  if (result != 0) {
    git_signature_free(au);
    return false;
  }
  git_oid newoid;
  std::string refname = std::string("refs/heads/") + opt.branch;
  if (git_commit_create(&newoid, repo, refname.c_str(), au, cu, nullptr,
                        opt.message.c_str(), tree, 0, nullptr) != 0) {
    auto e = giterr_last();
    aze::FPrintF(stderr, "create new branch %s failed: %s\n",
                 opt.branch.c_str(), e->message);
    git_signature_free(au);
    git_signature_free(cu);
    return false;
  }
  aze::FPrintF(stderr, "[%s %s]\ncommitter: %s\nemail: %s\nmessage: %s\n\n",
               opt.branch, git_oid_tostr_s(&newoid), cu->name, cu->email,
               opt.message);
  git_signature_free(au);
  git_signature_free(cu);
  return true;
}

bool cheat_execute(cheat_options &opt) {
  // TODO initialize libgit2 thread safe.
  git::global_initializer_t gi;
  if (opt.branch.empty()) {
    aze::FPrintF(stderr, "New branch name cannot be empty.\n");
    return false;
  }

  if (opt.gitdir.empty()) {
    opt.gitdir = ".";
  }
  if (opt.parent.empty()) {
    opt.parent = "HEAD"; /// current head
  }
  git::error_code ec;
  auto r = git::repository::make_repository_ex(opt.gitdir, ec);
  if (!r) {
    aze::FPrintF(stderr, "Error: %s\n", ec.message);
    return false;
  }
  if (r->get_branch(opt.branch)) {
    aze::FPrintF(stderr,
                 "Branch '%s' exists, please create other name branch.\n",
                 opt.branch);
    return false;
  }
  auto c = r->get_reference_commit_auto(opt.parent);
  if (!c) {
    auto e = giterr_last();
    aze::FPrintF(stderr, "Error: %s\n", e->message);
    return false;
  }
  auto t = git::tree::get_tree(*r, *c, opt.treedir);
  if (!t) {
    auto e = giterr_last();
    aze::FPrintF(stderr, "Error: %s\n", e->message);
    return false;
  }
  if (opt.kauthor) {
    return duplicate_new_branch(r->p(), c->p(), t->p(), opt.branch);
  }
  if (opt.message.empty()) {
    opt.message = git_commit_message(c->p());
  }
  return hack_new_branch(r->p(), t->p(), opt);
}
