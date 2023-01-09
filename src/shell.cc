/**
 * Universidad de La Laguna
 * Escuela Superior de Ingeniería y Tecnología
 * Grado en Ingeniería Informática
 * Asignatura: Sistemas Operativos
 * Curso: 2º
 * Práctica 2: SHELL PROJECT
 * @autor: Valeria Bosch Pérez (alu0101485287@ull.edu.es)
 * @date: 3 Jan 2023
 * @file: shell.cc
 * @brief: shell class functions
 * Referencias:
 * Enlaces de interés
 */

#include <sys/wait.h>

#include "shell.h"
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
    for (int i = 1; i < args.size(); ++i) {
      std::cout << args[i] << " ";
    }
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
    if (commands[0] == "exit") return CommandResult::Quit();
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
  return CommandResult::Quit();
}

int Shell::ExecuteProgram(const std::vector<std::string>& args, bool has_wait = true) {
  try {
    pid_t pid = fork();
    if (pid < 0) std::throw_with_nested(std::runtime_error("ERROR: Creating the process!"));
    if (pid > 0) {
      if (has_wait) {
        int status;
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
      } else {
        return pid;
      }
    } else {
      char** argv = new char*[args.size() + 1];
      for (int i = 0; i < args.size(); i++) {
        argv[i] = const_cast<char*>(args[i].c_str());
      }
      argv[args.size()] = nullptr;
      if (execvp(argv[0], argv) < 0) exit(EXIT_FAILURE);
    }
    return 0;
  } catch (const std::exception& error) {
    std::throw_with_nested(std::runtime_error("ERROR: Executing function failed!"));
  }
}

void PrintError(const std::string& error) {
  std::cerr << "ERROR: " << error << '\n' << '\n';
  std::cerr.flush();
}

void Shell::Run() {
  std::vector<std::vector<std::string>> commands;
  std::string line;
  int last_command_status = 0;
  // Bucle principal de la SHELL
  while (true) {
    try {
      // Imprimir el prompt
      PrintPrompt(last_command_status);
      // Lee la línea 
      line = ReadLine(STDIN_FILENO);
      // Divide la entrada en comandos
      commands = ParseLine(line);
      if (line.empty()) continue;
      for (const auto& cmd : commands) {
        auto [return_value, is_quit_requested] = ExecuteCommand(cmd);
        if (is_quit_requested) exit(EXIT_SUCCESS);
        std::cout << std::endl;
        last_command_status = return_value;
        if (return_value != 0) PrintError("ERROR: Executing command failed!");
      }
    } catch (const std::exception& error) {
      std::cout << std::endl;
      PrintError(error.what());
      last_command_status = 1;
    }
  }
}