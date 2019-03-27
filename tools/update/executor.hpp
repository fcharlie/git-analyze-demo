///////////
#ifndef EXECUTOR_HPP
#define EXECUTOR_HPP
#include "engine.hpp"

class executor_base;
class Executor {
public:
  Executor();
  ~Executor();
  Executor(const Executor &) = delete;
  Executor &operator=(const Executor &) = delete;
  bool InitializeRules(std::string_view sv, std::string_view ref);
  bool Execute(std::string_view path, std::string_view oldrev,
               std::string_view newrev); // GIT_DIR

  // FullMatch
  bool FullMatch(std::string_view path) { return engine.FullMatch(path); }

private:
  bool ExecuteTreeWalk(std::string_view rev);
  RulesEngine engine;
  executor_base *base{nullptr};
};

#endif
