///////
#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <cstdio>
#include <vector>
#include <string_view>
#include <re2/re2.h>

// https://github.com/google/re2/wiki/Syntax
class RulesEngine {
public:
  RulesEngine() = default;
  RulesEngine(const RulesEngine &) = delete;
  RulesEngine &operator=(const RulesEngine &) = delete;
  ~RulesEngine() {
    for (auto c : rules) {
      delete c;
    }
    rules.clear();
  }
  bool AddPrefix(std::string_view path);
  bool AddRegex(std::string_view rx);
  bool FullMatch(std::string_view path);
  bool PreInitialize(std::string_view jf, std::string_view branch);

private:
  std::vector<std::string> prefix;
  std::vector<RE2 *> rules;
};

#endif
