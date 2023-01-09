#ifndef USAGES_H
#define USAGES_H

#include <exception>

void Usage(const int argc, const char* argv[]);
void Program(const int argc, const char* argv[]);
void PrintException(const std::exception& error);

#endif