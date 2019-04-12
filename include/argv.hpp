////
#ifndef AZE_PARSEARGV_HPP
#define AZE_PARSEARGV_HPP
#include <string>
#include <vector>
#include <functional>
#include <string_view> // C++17
#include <absl/strings/str_cat.h>
//
namespace av {
enum ParseError {
  SkipParse = -1,
  None = 0,
  ErrorNormal = 1 //
};
struct error_code {
  std::string message;
  int ec{0};
  explicit operator bool() const noexcept { return ec != None; }
  void Assign(std::string_view sv, int val = ErrorNormal) {
    ec = val;
    message.assign(sv.data(), sv.size());
  }
  template <typename... Args> void Assign(int val, Args... args) {
    ec = val;
    std::initializer_list<std::string_view> as = {
        static_cast<const absl::AlphaNum &>(args).Piece()...};
    message = absl::strings_internal::CatPieces(as);
  }
};

// Nomal make_error_code
inline error_code make_error_code(std::string_view m, int ec = ErrorNormal) {
  return error_code{std::string(m.data(), m.size()), ec};
}

template <typename... Args> error_code make_error_code_v(int ec, Args... args) {
  std::initializer_list<std::string_view> as = {
      static_cast<const absl::AlphaNum &>(args).Piece()...};
  return error_code{absl::strings_internal::CatPieces(as), ec};
}

enum HasArgs {
  required_argument, /// -i 11 or -i=xx
  no_argument,
  optional_argument /// -s --long --long=xx
};
struct option {
  std::string_view name;
  HasArgs has_args;
  int val;
};
using invoke_t = std::function<bool(int, const char *, const char *)>;
class ParseArgv {
public:
  using StringArray = std::vector<std::string_view>;
  ParseArgv(int argc, char *const *argv) : argc_(argc), argv_(argv) {}
  ParseArgv(const ParseArgv &) = delete;
  ParseArgv &operator=(const ParseArgv &) = delete;
  ParseArgv &Add(std::string_view name, HasArgs a, int val) {
    options_.push_back({name, a, val});
    return *this;
  }
  bool Execute(const invoke_t &v, error_code &ec);
  const StringArray &UnresolvedArgs() const { return uargs; }

private:
  int argc_;
  char *const *argv_;
  int index{0};
  StringArray uargs;
  std::vector<option> options_;
  bool parse_internal(std::string_view a, const invoke_t &v, error_code &ec);
  bool parse_internal_long(std::string_view a, const invoke_t &v,
                           error_code &ec);
  bool parse_internal_short(std::string_view a, const invoke_t &v,
                            error_code &ec);
};

// ---------> parse internal
inline bool ParseArgv::Execute(const invoke_t &v, error_code &ec) {
  if (argc_ == 0 || argv_ == nullptr) {
    ec.Assign("the command line array is empty.");
    return false;
  }
  index = 1;
  for (; index < argc_; index++) {
    std::string_view a = argv_[index];
    if (a.empty() || a.front() != '-') {
      uargs.push_back(a);
      continue;
    }
    // parse ---
    if (!parse_internal(a, v, ec)) {
      return false;
    }
  }
  return true;
}

inline bool ParseArgv::parse_internal_short(std::string_view a,
                                            const invoke_t &v, error_code &ec) {
  int ch = -1;
  HasArgs ha = optional_argument;
  const char *oa = nullptr;
  // -x=XXX
  // -xXXX
  // -x XXX
  // -x; BOOL
  if (a[0] == '=') {
    ec.Assign(1, "unexpected argument '-", a, "'"); // -=*
    return false;
  }
  auto c = a[0];
  for (const auto &o : options_) {
    if (o.val == c) {
      ch = o.val;
      ha = o.has_args;
      break;
    }
  }
  if (ch == -1) {
    ec.Assign(1, "unregistered option '-", a, "'");
    return false;
  }
  // a.size()==1 'L' short value
  if (a.size() >= 2) {
    oa = (a[1] == '=') ? (a.data() + 2) : (a.data() + 1);
  }
  if (oa != nullptr && ha == no_argument) {
    ec.Assign(1, "option '-", a.substr(0, 1), "' unexpected parameter: ", oa);
    return false;
  }
  if (oa == nullptr && ha == required_argument) {
    if (index + 1 >= argc_) {
      ec.Assign(1, "option '-", a, "' missing parameter");
      return false;
    }
    oa = argv_[index + 1];
    index++;
  }
  if (!v(ch, oa, a.data())) {
    ec.Assign(SkipParse, "skip parse");
    return false;
  }
  return true;
}

// Parse long option
inline bool ParseArgv::parse_internal_long(std::string_view a,
                                           const invoke_t &v, error_code &ec) {
  // --xxx=XXX
  // --xxx XXX
  // --xxx; bool
  int ch = -1;
  HasArgs ha = optional_argument;
  const char *oa = nullptr;
  auto pos = a.find('=');
  if (pos != std::string_view::npos) {
    if (pos + 1 >= a.size()) {
      ec.Assign(1, "unexpected argument '--", a, "'");
      return false;
    }
    oa = a.data() + pos + 1;
    a.remove_suffix(a.size() - pos);
  }
  for (const auto &o : options_) {
    if (o.name == a) {
      ch = o.val;
      ha = o.has_args;
      break;
    }
  }
  if (ch == -1) {
    ec.Assign(1, "unregistered option '--", a, "'");
    return false;
  }
  if (oa != nullptr && ha == no_argument) {
    ec.Assign(1, "option '--", a, "' unexpected parameter: ", oa);
    return false;
  }
  if (oa == nullptr && ha == required_argument) {
    if (index + 1 >= argc_) {
      ec.Assign(1, "option '--", a, "' missing parameter");
      return false;
    }
    oa = argv_[index + 1];
    index++;
  }
  if (!v(ch, oa, a.data())) {
    ec.Assign(SkipParse, "skip parse");
    return false;
  }
  return true;
}

inline bool ParseArgv::parse_internal(std::string_view a, const invoke_t &v,
                                      error_code &ec) {
  if (a.size() == 1) {
    ec.Assign("unexpected argument '-'");
    return false;
  }
  if (a[1] == '-') {
    return parse_internal_long(a.substr(2), v, ec);
  }
  return parse_internal_short(a.substr(1), v, ec);
}

} // namespace av

#endif
