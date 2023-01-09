#ifndef SCOPE_EXIT_H
#define SCOPE_EXIT_H

#include <functional>

class ScopeExit {
 public:
  explicit ScopeExit(const std::function<void()>& scope_exit) : scope_exit_(scope_exit) { }
  ~ScopeExit() { scope_exit_(); }
 private:
  std::function<void()> scope_exit_;
};

#endif