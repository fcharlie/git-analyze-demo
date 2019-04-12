#ifndef ARGVEX_HPP
#define ARGVEX_HPP
#include <cstring>
#include <climits>
#include <vector>
#include <string_view>
#include <functional>
#include <absl/strings/str_cat.h>

namespace ax {
enum ParseError {
  SkipParse = -1,
  None = 0,
  ErrorNormal = 1 //
};

struct error_code {
  std::string message;
  int ec{0};
  operator bool() { return ec != 0; }
};

inline error_code make_error_code(absl::string_view m, int ec = ErrorNormal) {
  return error_code{std::string(m.data(), m.size()), ec};
}

template <typename... Args> error_code make_error_code_v(int ec, Args... args) {
  std::initializer_list<absl::string_view> as = {
      static_cast<const absl::AlphaNum &>(args).Piece()...};
  return error_code{absl::strings_internal::CatPieces(as), ec};
}

inline unsigned char _Digit_from_char(const char _Ch) noexcept // strengthened
{ // convert ['0', '9'] ['A', 'Z'] ['a', 'z'] to [0, 35], everything else to 255
  static constexpr unsigned char _Digit_from_byte[] = {
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   255, 255,
      255, 255, 255, 255, 255, 10,  11,  12,  13,  14,  15,  16,  17,  18,  19,
      20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  32,  33,  34,
      35,  255, 255, 255, 255, 255, 255, 10,  11,  12,  13,  14,  15,  16,  17,
      18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  32,
      33,  34,  35,  255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
      255};
  static_assert(std::size(_Digit_from_byte) == 256);

  return (_Digit_from_byte[static_cast<unsigned char>(_Ch)]);
}

template <typename Integer>
error_code Integer_from_chars(std::string_view wsv, Integer &ov,
                              const int base) {
  bool msign = false;
  auto _begin = wsv.begin();
  auto _end = wsv.end();
  auto _iter = _begin;
  if constexpr (std::is_signed_v<Integer>) {
    if (_iter != _end && *_iter == '-') {
      msign = true;
      _iter++;
    }
  }
  using Unsigned = std::make_unsigned_t<Integer>;
  constexpr Unsigned _Uint_max = static_cast<Unsigned>(-1);

  Unsigned _risky_val;
  Unsigned _max_digit;

  if constexpr (std::is_signed_v<Integer>) {
    constexpr Unsigned _Int_max = static_cast<Unsigned>(_Uint_max >> 1);
    if (msign) {
      constexpr Unsigned _Abs_int_min = static_cast<Unsigned>(_Int_max + 1);
      _risky_val = static_cast<Unsigned>(_Abs_int_min / base);
      _max_digit = static_cast<Unsigned>(_Abs_int_min % base);
    } else {
      _risky_val = static_cast<Unsigned>(_Int_max / base);
      _max_digit = static_cast<Unsigned>(_Int_max % base);
    }
  } else {
    _risky_val = static_cast<Unsigned>(_Uint_max / base);
    _max_digit = static_cast<Unsigned>(_Uint_max % base);
  }

  Unsigned _value = 0;

  bool _overflowed = false;

  for (; _iter != _end; ++_iter) {
    char wch = *_iter;
    if (wch > CHAR_MAX) {
      return make_error_code("out of range");
    }
    const unsigned char _digit = _Digit_from_char(static_cast<char>(wch));

    if (_digit >= base) {
      break;
    }

    if (_value < _risky_val // never overflows
        || (_value == _risky_val &&
            _digit <= _max_digit)) // overflows for certain digits
    {
      _value = static_cast<Unsigned>(_value * base + _digit);
    } else // _Value > _Risky_val always overflows
    {
      _overflowed = true; // keep going, _Next still needs to be updated, _Value
                          // is now irrelevant
    }
  }

  if (_iter - _begin == static_cast<ptrdiff_t>(msign)) {
    return make_error_code("invalid argument");
  }

  if (_overflowed) {
    return make_error_code("result out of range");
  }

  if constexpr (std::is_signed_v<Integer>) {
    if (msign) {
      _value = static_cast<Unsigned>(0 - _value);
    }
  }
  ov = static_cast<Unsigned>(_value); // implementation-defined for negative,
                                      // N4713 7.8 [conv.integral]/3
  return error_code{};
}

class ParseArgv {
public:
  ParseArgv(int argc, char *const *argv) : argc_(argc), argv_(argv) {
    ///
  }
  ParseArgv(const ParseArgv &) = delete;
  ParseArgv &operator=(const ParseArgv &) = delete;
  enum HasArgs {
    required_argument, /// -i 11 or -i=xx
    no_argument,
    optional_argument /// -s --long --long=xx
  };
  struct option {
    const char *name;
    HasArgs has_args;
    int val;
  };
  // int ch,const char *optarg, const char *raw
  using callback_t = std::function<bool(int, const char *, const char *)>;
  using options_t = std::vector<option>;
  /////////// Parse
  error_code Parse(const options_t &opts, const callback_t &callback) {
    if (argc_ == 0 || argv_ == nullptr) {
      return make_error_code("bad argv input");
    };
    index = 1;
    for (; index < argc_; index++) {
      std::string_view arg = argv_[index];
      if (arg[0] != '-') {
        uargs.push_back(arg);
        continue;
      }
      auto ec = parse_internal(arg, opts, callback);
      if (ec) {
        return ec;
      }
    }
    return error_code{};
  }
  const std::vector<std::string_view> &UnresolvedArgs() const { return uargs; }

private:
  int argc_;
  char *const *argv_;
  std::vector<std::string_view> uargs;
  int index{0};
  error_code parse_internal(std::string_view arg, const options_t &opts,
                            const callback_t &callback);
};

inline error_code ParseArgv::parse_internal(std::string_view arg,
                                            const options_t &opts,
                                            const callback_t &callback) {
  /*
  -x ; -x value -Xvalue
  --xy;--xy=value;--xy value
  */
  if (arg.size() < 2) {
    return make_error_code("invaild argument '-'");
  }

  int ch = -1;
  HasArgs ha = optional_argument;
  const char *optarg = nullptr;

  if (arg[1] == '-') {
    /// parse long
    /// --name value; --name=value
    std::string_view name;
    auto pos = arg.find('=');
    if (pos != std::string_view::npos) {
      if (pos + 1 >= arg.size()) {
        return make_error_code_v(1, "incorrect argument: ", arg);
      }
      name = arg.substr(2, pos - 2);
      optarg = arg.data() + pos + 1;
    } else {
      name = arg.substr(2);
    }
    for (auto &o : opts) {
      if (name.compare(o.name) == 0) {
        ch = o.val;
        ha = o.has_args;
        break;
      }
    }
    if (ch == -1) {
      return make_error_code_v(1, "unregistered option: ", arg);
    }
  } else {

    /// -x=xxx
    if (arg.size() == 3 && arg[2] == '=') {
      return make_error_code_v(1, "incorrent argument: ", arg);
    }
    if (arg.size() > 3) {
      if (arg[2] == '=') {
        optarg = arg.data() + 3;
      } else {
        optarg = arg.data() + 2;
      }
    }
    for (auto &o : opts) {
      if (o.val == arg[1]) {
        ha = o.has_args;
        ch = o.val;
        break;
      }
    }
    if (ch == -1) {
      return make_error_code_v(1, "unregistered option: ", arg);
    }
  }
  if (optarg != nullptr && ha == no_argument) {
    return make_error_code_v(1, "unacceptable input: ", arg);
  }
  if (optarg == nullptr && ha == required_argument) {
    if (index + 1 >= argc_) {
      return make_error_code_v(1, "option name cannot be empty: ", arg);
    }
    optarg = argv_[index + 1];
    index++;
  }
  if (callback(ch, optarg, arg.data())) {
    return error_code{};
  }
  return make_error_code("parse skip", SkipParse);
}
} // namespace ax

#endif
