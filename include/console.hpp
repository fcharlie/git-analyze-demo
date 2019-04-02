//////////
#ifndef AZE_CONSOLE_HPP
#define AZE_CONSOLE_HPP
#include <string>
#include <string_view>

#ifndef _WIN32
#include "impl/console_unix.hpp"
#else
#include "impl/console_win.hpp"
#endif

#endif
