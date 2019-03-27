//////////////
#ifndef GIT_ANALYZE_OS_HPP
#define GIT_ANALYZE_OS_HPP
#include <string>

namespace os {
bool Executable(std::string &exe);
bool PathRemoveFileSpec(std::string &path);
std::string Getwd();
} // namespace os

#ifdef _WIN32
#include "impl/os_windows.hpp"
#else
#include "impl/os_unix.hpp"
#endif

#endif
