#ifndef SHELL_SYSTEM_H
#define SHELL_SYSTEM_h

#include <iostream>
#include <exception>
#include <fcntl.h>
#include <stdexcept>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>
#include <sstream>
#include <vector>
#include <utime.h>
#include <pwd.h>
#include <regex>
#include <vector>
#include <string>

struct CommandResult {
  int return_value;
  bool is_quit_requested;
  CommandResult(int return_value, bool request_quit = false) : return_value{return_value}, is_quit_requested{request_quit} {}

  static CommandResult Quit(int return_value = 0) {
    return CommandResult{return_value, true};
  };
};

std::vector<uint8_t> ReadFile(const int fd);
std::vector<uint8_t> WriteFile(int fd, std::vector<uint8_t> buffer);
void CopyFile(const std::string& src_path, const std::string& dst_path, bool preserve_all);
void MoveFile(const std::string& src_path, const std::string& dst_path);
std::vector<std::string> SplitSpaces(const std::string& input_string);
void PrintLine(const std::string& output_string);
void PrintPrompt(int last_command_status);
std::string ReadLine(int fd);
std::vector<std::vector<std::string>> ParseLine(const std::string& line);

#endif