#include "shell.h"
#include "shell-system.h"
#include "usages.h"

int Shell::CdCommand(const std::vector<std::string>& args) {
  try {
    if (args.size() > 2) std::throw_with_nested(std::runtime_error("ERROR: Too many arguments!"));
    if (args.size() == 1) {
      const char* home_directory;
      if ((home_directory = getenv("HOME")) == NULL) home_directory = getpwuid(getuid())->pw_dir;
      chdir(home_directory);
    } else {
      if (chdir(args[1].c_str()) != 0) throw std::system_error(errno, std::system_category());
    }
  } catch (const std::system_error& error) {
    std::throw_with_nested(std::runtime_error("ERROR: cd command failed!"));
    return 1;
  }
  return 0;
}

int Shell::CpCommand(const std::vector<std::string>& args) {
  try {
    bool copy_attributes = false;
    bool move_file = false;
    for (const auto& parameter : args) {
      if (parameter == "-m") {
        move_file = true;
        break;
      }
      if (parameter == "-a") {
        copy_attributes = true;
        break;
      } 
    }
    int shift = 0;
    if (args.size() == 4) shift += 1;
    bool preserve_all = false;
    if (copy_attributes || move_file) preserve_all = true;
    std::string src_path = args[1 + shift];
    std::string dst_path = args[2 + shift];
    if (!move_file) CopyFile(src_path, dst_path, preserve_all);
    else MvCommand(args);
  } catch (const std::system_error& error) {
    std::throw_with_nested(std::runtime_error("ERROR: cp command failed!"));
    return 1;
  }
  return 0;
}

int Shell::MvCommand(const std::vector<std::string>& args) {
  try {
    int shift = 0;
    if (args.size() == 4) shift += 1;
    std::string src_path = args[1 + shift];
    std::string dst_path = args[2 + shift];
    MoveFile(src_path, dst_path);
  } catch (const std::system_error& error) {
    std::throw_with_nested(std::runtime_error("ERROR: mv command failed!"));
    return 1;
  }
  return 0;
}

int Shell::EchoCommand(const std::vector<std::string>& args) {
  try {
    bool in_quotes = false;
    std::stringstream output;
    for (int i = 1; i < args.size(); ++i) {
      std::string arg = args[i];
      if (arg[0] == '"') {
        if (arg[arg.size() - 1] == '"') {
          arg = arg.substr(1, arg.size() - 2);
        } else {
          in_quotes = true;
          arg = arg.substr(1);
        }
      }
      if (in_quotes) {
        if (arg[arg.size() - 1] == '"') {
          in_quotes = false;
          arg = arg.substr(0, arg.size() - 1);
        }
        while (in_quotes && i < args.size() - 1) {
          arg += ' ' + args[++i];
          if (arg[arg.size() - 1] == '"') {
            in_quotes = false;
            arg = arg.substr(0, arg.size() - 1);
          }
        }
      }
      if (in_quotes || arg[arg.size() - 1] == '"') {
        std::throw_with_nested(std::runtime_error("ERROR: Unbalanced quotes"));
        return 1;
      }
      output << arg << " ";
    }
    output << std::endl;
    std::cout << output.str();
  } catch (const std::system_error& error) {
    std::throw_with_nested(std::runtime_error("ERROR: echo command failed!"));
  }
  return 0;
}

CommandResult Shell::ExecuteCommand(const std::vector<std::string>& commands) {
  try {
    bool incorrect_command = true;
    for (const auto& internal_command : internal_commands_) {
      if (commands[0] == internal_command) {
        incorrect_command = false;
        break;
      }
    }
    //if (incorrect_command) std::throw_with_nested(std::runtime_error("The introduced command was unexpected"));
    if (commands[0] == "exit") return CommandResult::quit();
    else if (commands[0] == "cp") {
      return CommandResult(CpCommand(commands), false);
    } else if (commands[0] == "cd") {
      return CommandResult(CdCommand(commands), false);
    } else if (commands[0] == "mv") {
      return CommandResult(MvCommand(commands), false);
    } else if (commands[0] == "echo") {
      return CommandResult(EchoCommand(commands), false);
    } else {
      return CommandResult(ExecuteProgram(commands, true), false);
    }
  } catch (const std::exception& error) {
    std::throw_with_nested(std::runtime_error("ERROR: Executing commands failed!"));
  }
  return CommandResult::quit();
}

void PrintError(const std::string& error) {
  std::cerr << "ERROR: " << error << '\n' << '\n';
  std::cerr.flush();
}

void Shell::Run() {
  std::vector<std::vector<std::string>> commands;
  std::string line;
  int last_command_status = 0;
  while (true) {
    try {
      PrintPrompt(last_command_status);
      line = ReadLine(STDIN_FILENO);
      commands = ParseLine(line);
      if (line.empty()) continue;
      for (const auto& cmd : commands) {
        auto [return_value, is_quit_requested] = ExecuteCommand(cmd);
        std::cout << std::endl;
        if (is_quit_requested) exit(EXIT_SUCCESS);
        last_command_status = return_value;
        if (return_value != 0) PrintError("ERROR: Executing command failed!");
      }
    } catch (const std::exception& error) {
      PrintError(error.what());
      last_command_status = 1;
    }
  }
}