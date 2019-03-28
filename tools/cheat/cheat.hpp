///
#ifndef GIT_CHEAT_HPP
#define GIT_CHEAT_HPP
#include <string>
#include <ctime>

struct cheat_options {
  std::string gitdir;        // gitdir pos 0
  std::string branch;        // branch pos 1
  std::string message;       // commit message pos 2
  std::string parent;        // parent branch or commit
  std::string treedir;       // treedir
  std::string author;        // author
  std::string authoremail;   //
  std::string committer;     // commiter
  std::string commiteremail; ///
  std::time_t date{0};
  int timeoff{0};
  bool verbose{false};
  bool kauthor{false};
};

bool cheat_execute(cheat_options &opt);

#endif
