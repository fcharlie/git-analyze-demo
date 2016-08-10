#include <cstdio>
#include <string>
#include <sys/stat.h>

#ifdef _WIN32
bool PathSearchAuto(const char *cmd) { return true; }
#else

bool PathSearchAuto(const char *cmd) {
  auto path_ = getenv("PATH");
  std::string gitbin;
  for (; *path_; path_++) {
    if (*path_ == ':') {
      gitbin.push_back('/');
      gitbin.append(cmd);
      struct stat st;
      if (stat(gitbin.c_str(), &st) == 0) {
        fprintf(stderr, "%s\n", gitbin.c_str());
        return true;
      }
      gitbin.clear();
    } else {
      gitbin.push_back(*path_);
    }
  }
  return false;
}
#endif
int main() {
  /* code */
  if (PathSearchAuto("git")) {
    printf("Find git success \n");
  }
  return 0;
}
