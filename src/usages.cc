#include <iostream>
#include <libgen.h>
#include <filesystem>
#include <vector>
#include <string>
#include <sstream>
#include <exception>

#include "shell-system.h"
#include "shell.h"
#include "usages.h"

void PrintException(const std::exception& error) {
  std::cerr << "ERROR: " << error.what() << '\n';
  try {
    std::rethrow_if_nested(error);
  } catch(const std::exception& nested_exception) {
    PrintException(nested_exception);
  } catch(...) {}
}

void Program(const int argc, const char* argv[]) {
  std::vector<std::string> args(argv, argv + argc);
  try {
    system("clear");
    Shell shell(0);
    shell.Run();
  } catch(...) {
    std::stringstream error;
    error << "\x1b[1;33mTry " << args[0] << " --help for more information\x1b[0m";
    std::throw_with_nested(std::runtime_error(error.str()));
  }
}