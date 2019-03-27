//
#ifndef GA_OS_UNIX_HPP
#define GA_OS_UNIX_HPP

#include <cassert>
#include <cstdint>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <pwd.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#if defined(__FreeBSD__)
#include <sys/types.h>
#include <sys/param.h>
#endif
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif
#if defined(__GNU__) && !defined(PATH_MAX)
#define PATH_MAX 4096
#endif

#if defined(__linux__)
#define symlinkEntrypointExecutable "/proc/self/exe"
#elif !defined(__APPLE__)
#define symlinkEntrypointExecutable "/proc/curproc/exe"
#endif

namespace os {

inline bool Executable(std::string &exe) {
  bool result = false;

  exe.clear();

// Get path to the executable for the current process using
// platform specific means.
#if defined(__linux__) || (defined(__NetBSD__) && !defined(KERN_PROC_PATHNAME))
  // On Linux, fetch the entry point EXE absolute path, inclusive of filename.
  char myexe[PATH_MAX];
  ssize_t res = readlink(symlinkEntrypointExecutable, myexe, PATH_MAX - 1);
  if (res != -1) {
    myexe[res] = '\0';
    exe.assign(myexe);
    result = true;
  } else {
    result = false;
  }
#elif defined(__APPLE__)

  // On Mac, we ask the OS for the absolute path to the entrypoint executable
  uint32_t lenActualPath = 0;
  if (_NSGetExecutablePath(nullptr, &lenActualPath) == -1) {
    // OSX has placed the actual path length in lenActualPath,
    // so re-attempt the operation
    std::string resizedPath(lenActualPath, '\0');
    char *pResizedPath = const_cast<char *>(resizedPath.c_str());
    if (_NSGetExecutablePath(pResizedPath, &lenActualPath) == 0) {
      exe.assign(pResizedPath);
      result = true;
    }
  }
#elif defined(__FreeBSD__)
  static const int name[] = {CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1};
  char path[PATH_MAX];
  size_t len;

  len = sizeof(path);
  if (sysctl(name, 4, path, &len, nullptr, 0) == 0) {
    exe.assign(path);
    result = true;
  } else {
    // ENOMEM
    result = false;
  }
#elif defined(__NetBSD__) && defined(KERN_PROC_PATHNAME)
  static const int name[] = {
      CTL_KERN,
      KERN_PROC_ARGS,
      -1,
      KERN_PROC_PATHNAME,
  };
  char path[MAXPATHLEN];
  size_t len;

  len = sizeof(path);
  if (sysctl(name, __arraycount(name), path, &len, NULL, 0) != -1) {
    exe.assign(path);
    result = true;
  } else {
    result = false;
  }
#else
  // On non-Mac OS, return the symlink that will be resolved by GetAbsolutePath
  // to fetch the entrypoint EXE absolute path, inclusive of filename.
  exe.assign(symlinkEntrypointExecutable);
  result = true;
#endif

  return result;
}

inline bool PathRemoveFileSpec(std::string &path) {
  if (path.empty()) {
    return false;
  }
  auto p = path.c_str();
  auto end = p + path.size();
  while (p < end--) {
    if (*end == '/') {
      auto l = end - p;
      path.erase(l == 0 ? 1 : l);
      return true;
    }
  }
  return false;
}
constexpr const char PathSeparator = '/';
}; // namespace os

#endif
