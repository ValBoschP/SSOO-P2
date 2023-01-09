#include <iostream>

#include "usages.h"
#include "shell-system.h"

int main(const int argc, const char* argv[]) {
  try {
    Usage(argc, argv);
    Program(argc, argv);
  } catch (const std::exception& error) {
    PrintException(error);
  }
  return 0;
}