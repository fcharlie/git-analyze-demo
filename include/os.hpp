//////////////
#ifndef GIT_ANALYZE_OS_HPP
#define GIT_ANALYZE_OS_HPP
#include <string>
#include <string_view>
#include <optional>

namespace os {
bool Executable(std::string &exe);
bool PathRemoveFileSpec(std::string &path);
std::string PathClean(std::string_view sv);
std::string Getwd();
std::optional<std::string> Gitdir(std::string_view sv);
} // namespace os

#ifdef _WIN32
#include "impl/os_windows.hpp"
#else
#include "impl/os_unix.hpp"
#endif

#endif
