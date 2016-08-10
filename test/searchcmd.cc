#include <cstdio>
#include <cstring>
#include <string>
#include <sys/stat.h>

#ifdef _WIN32
#include <Windows.h>
bool PathSearchAuto(const char *cmd) {
  struct stat st;
  if (stat(cmd, &st) == 0) {
    fprintf(stderr, "%s\n", gitbin.c_str());
    return true;
  }
  auto path_ = getenv("PATH");
  std::string gitbin;
  for (; *path_; path_++) {
    if (*path_ == ';') {
      gitbin.push_back('\\');
      gitbin.append(cmd);
      if (stat(gitbin.c_str(), &st) == 0) {
        fprintf(stderr, "%s\n", gitbin.c_str());
        return true;
      }
      gitbin.clear();
    } else {
      gitbin.push_back(*path_);
    }
  }
  char buf[MAX_PATH] = {0};
  if (!GetModuleFileNameA(nullptr, buf, MAX_PATH)) {
    return false;
  }
  auto end = buf + strlen(buf);
  for (; end > buf; end--) {
    if (end == '/' || end == '\\') {
      *end = 0;
      break;
    }
  }
  gitbin.assign(buf);
  gitbin.push_back('/');
  gitbin.append(cmd);
  ///// support self directory because Windows support current dir start
  if (stat(gitbin.c_str(), &st) == 0) {
    fprintf(stderr, "%s\n", gitbin.c_str());
    return true;
  }
  return true;
}
#else

bool PathSearchAuto(const char *cmd) {
  struct stat st;
  // full path /path/to/cmd
  if (*cmd == '/') {
    if (stat(cmd, &st) == 0) {
      return true;
    }
  }

  auto l = strlen(cmd);
  // ./path/to/cmd
  if (l >= 3 && cmd[0] == '.' & cmd[1] == '/') {
    if (stat(cmd, &st) == 0) {
      return true;
    }
  }
  // ../path/to/cmd
  if (l >= 4 && cmd[0] == '.' && cmd[1] == '.' && cmd[2] == '/') {
    if (stat(cmd, &st) == 0) {
      return true;
    }
  }
  auto path_ = getenv("PATH");
  std::string gitbin;
  for (; *path_; path_++) {
    if (*path_ == ':') {
      gitbin.push_back('/');
      gitbin.append(cmd);
      struct stat st;
      if (stat(gitbin.c_str(), &st) == 0 && S_ISREG(st.st_mode)) {
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
////
int main() {
  /* code */
  if (PathSearchAuto("git")) {
    printf("Find git success \n");
  } else {
    fprintf(stderr, "Not Found Git in your path \n");
  }
  return 0;
}
