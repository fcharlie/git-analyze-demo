/*
 * commit.cc
 * git-analyze
 * author: Force.Charlie
 * Date: 2017.04
 * Copyright (C) 2019. GITEE.COM. All Rights Reserved.
 */
#include <ctime>
#include "complete.hpp"

bool Executor::Parseconfig() {
  auto c0 = git::config::global();
  if (c0) {
    auto a = c0->get("user.name");
    if (!a.empty()) {
      author.assign(a);
    }
    auto e = c0->get("user.email");
    if (!e.empty()) {
      email.assign(e);
    }
  }
  auto c1 = r.get_config();
  if (c1) {
    auto a = c0->get("user.name");
    if (!a.empty()) {
      author.assign(a);
    }
    auto e = c0->get("user.email");
    if (!e.empty()) {
      email.assign(e);
    }
  }
  auto m = getenv("GIT_COMMIT_MAXCOUNT");
  if (m != nullptr) {
    char *c;
    auto i = strtol(m, &c, 10);
    if (errno == 0 && i > 0 && i < 32) {
      maxcount = i;
    }
  }
  auto a = getenv("GIT_COMMIT_AUTHOR");
  if (a != nullptr) {
    author.assign(a);
  }
  auto e = getenv("GIT_COMMIT_EMAIL");
  if (e != nullptr) {
    author.assign(e);
  }
  return !(author.empty() || email.empty());
}

bool Executor::Initialize(std::string_view dir, std::string_view branch,
                          std::string_view message, bool nb_) {
#ifdef _WIN32
  _tzset();
#else
  tzset();
#endif
  git::error_code ec;
  auto xr = git::repository::make_repository_ex(dir, ec);
  if (!xr) {
    aze::FPrintF(stderr, "unable open '%s'\n", dir);
    return false;
  }
  r.acquire(std::move(*xr));
  if (!Parseconfig()) {
    aze::FPrintF(stderr, "Please set git commit email and git commit name\n");
    return false;
  }

  msg = message;
  if (absl::StartsWith(branch, "refs/heads/")) {
    refname = branch;
  } else {
    refname = absl::StrCat("refs/heads/", branch);
  }
  nb = nb_;
  if (nb) {
    auto ref = r.get_reference(refname);
    if (ref) {
      fprintf(stderr, "branch '%s' exists, unable create new branch\n",
              branch.data());
      return false;
    }
  }
  auto c = r.get_reference_commit_auto("HEAD");
  if (!c) {
    aze::FPrintF(stderr, "Unable resolve 'HEAD'\n");
    return false;
  }
  auto t_ = git::tree::get_tree(r, *c, "");
  if (!t_) {
    aze::FPrintF(stderr, "Unable resolve 'HEAD' tree\n");
    return false;
  }
  t.acquire(std::move(*t_));
  parent = c->lost(); // lost pointer
  return true;
}

bool Executor::One(git_time when) {
  git_signature sig;
  sig.name = author.data();
  sig.email = email.data();
  sig.when = when;
  git_oid nid;
  const git_commit *ps[] = {parent};
  if (git_commit_create(&nid, r.p(), refname.c_str(), &sig, &sig, nullptr,
                        msg.data(), t.p(), (nb ? 0 : 1),
                        (nb ? nullptr : ps)) != 0) {
    return false;
  }
  nb = false;
  if (parent != nullptr) {
    git_commit_free(parent);
    parent = nullptr;
  }
  totals++;
  return (git_commit_lookup(&parent, r.p(), &nid) == 0);
}

static uint32_t Random() {
  static uint32_t g_seed = 0;
  g_seed = (214013 * g_seed + 2531011);
  return (g_seed >> 16) & 0x7FFF;
}

bool Executor::RoundYear(int year) {
  auto t = time(nullptr);
  auto p = localtime(&t);
  unsigned my = year <= 1900 ? p->tm_year : year - 1900;
  struct tm mt;
  memset(&mt, 0, sizeof(tm));
  auto days = Days(year);
  git_time gt;
#ifdef _WIN32
  long tz = 0;
  _get_timezone(&tz);
  gt.offset = -tz / 60;
#else
  gt.offset = -timezone / 60;
#endif
  for (unsigned i = 1; i <= days; i++) {
    mt.tm_mday = i;
    mt.tm_mon = 0;
    constexpr uint32_t mincount = 2;
    auto N = (std::max)(Random() % maxcount, mincount);
    for (unsigned k = 0; k < N; k++) {
      mt.tm_hour = p->tm_hour;
      mt.tm_min = p->tm_min + k;
      mt.tm_sec = p->tm_sec;
      mt.tm_year = my;
      gt.time = mktime(&mt);
      if (!One(gt)) {
        return false;
      }
    }
  }
  return true;
}

bool Executor::Execute(uint32_t begin, uint32_t end) {
  for (auto i = begin; i <= end; i++) {
    if (!RoundYear(i)) {
      return false;
    }
    aze::FPrintF(stderr, "\rFill %04d done", i);
  }
  return true;
}
