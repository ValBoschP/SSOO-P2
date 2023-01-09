#ifndef SHELL_H
#define SHELL_H

#include <sys/types.h>
#include <string>
#include <map>
#include <functional>
#include <vector>

#include "shell-system.h"

class Shell {
 public:
 // Constructor
  Shell(const pid_t& procces_id) : procces_id_(procces_id) {}

// Getter
  inline const std::vector<std::string>& GetInternalCommands() const { return internal_commands_; }

// Methods
  int EchoCommand(const std::vector<std::string>& args);
  int CdCommand(const std::vector<std::string>& args);
  int CpCommand(const std::vector<std::string>& args);
  int MvCommand(const std::vector<std::string>& args);

  CommandResult ExecuteCommand(const std::vector<std::string>& commands);
  int ExecuteProgram(const std::vector<std::string>& args, bool has_wait);

  void Run();

 private:
  pid_t procces_id_;
  std::vector<std::string> internal_commands_ = { "cd", "echo", "cp", "mv", "exit" };
};

#endif