/**
 * Universidad de La Laguna
 * Escuela Superior de Ingeniería y Tecnología
 * Grado en Ingeniería Informática
 * Asignatura: Sistemas Operativos
 * Curso: 2º
 * Práctica 2: SHELL PROJECT
 * @autor: Valeria Bosch Pérez (alu0101485287@ull.edu.es)
 * @date: 3 Jan 2023
 * @file: main.cc
 * @brief: shell system functions
 * Referencias:
 * Enlaces de interés
 */

#include <iostream>

#include "usages.h"
#include "copyfile.h"

int main(const int argc, const char* argv[]) {
  try {
    Usage(argc, argv);
    Program(argc, argv);
  } catch (const std::exception& error) {
    PrintException(error);
  }
  return 0;
}