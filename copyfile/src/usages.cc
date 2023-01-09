/**
 * Universidad de La Laguna
 * Escuela Superior de Ingeniería y Tecnología
 * Grado en Ingeniería Informática
 * Asignatura: Sistemas Operativos
 * Curso: 2º
 * Práctica 2: SHELL PROJECT
 * @autor: Valeria Bosch Pérez (alu0101485287@ull.edu.es)
 * @date: 3 Jan 2023
 * @file: usages.cc
 * @brief: usages functions
 * Referencias:
 * Enlaces de interés
 */
#include <iostream>
#include <libgen.h>
#include <filesystem>
#include <vector>
#include <string>
#include <sstream>
#include <exception>

#include "usages.h"
#include "copyfile.h"

/**
 * @brief Imprime el uso del programa
 * @param argc El número de argumentos de línea de comando
 * @param argv El vector de argumentos de línea de comando
 */
void Usage(const int argc, const char* argv[]) {
  std::vector<std::string> args(argv, argv + argc);
  try {
    if (args.size() > 1 && (args[1] == "--help" || args[1] == "-h")) {
      std::cout << "      -- Copyfile --" << std::endl;
      std::cout << "HOW TO USE: " << args[0] << "[src] [dst]\n\n";
      std::cout << "[src]: The file to be copied\n";
      std::cout << "[dst]: The destination file where the file will be copied\n";
      std::cout << "\nPARAMETERS\n\n";
      std::cout << "-h: Shows this message\n";
      std::cout << "-m: Move the file instead of copying it\n";
      std::cout << "-a: Copy the attributes of the original file\n\n";
      exit(EXIT_SUCCESS);
    } 
    if (argc > 4 ) {
      std::filesystem::path exe_path = args[0];
      std::stringstream error_message;
      error_message << exe_path.filename().generic_string() << ": Invalid number of arguments!";
      throw std::runtime_error(error_message.str());
    }
  } catch (...) {
    std::stringstream error_message;
    error_message << "Try " << args[0] << " --help for more information";
    std::throw_with_nested(std::runtime_error(error_message.str()));
  }
}

/**
 * @brief Imprime un mensaje de excepción en el flujo de error estándar, junto con cualquier excepción anidada
 * @param error La excepción a imprimir
 */
void PrintException(const std::exception& error) {
  std::cerr << "ERROR: " << error.what() << '\n';
  try {
    std::rethrow_if_nested(error);
  } catch(const std::exception& nested_exception) {
    PrintException(nested_exception);
  } catch(...) {}
}

/**
 * @brief Ejecuta el programa principal
 * @param argc El número de argumentos de línea de comando
 * @param argv El vector de argumentos de línea de comando
 */
void Program(const int argc, const char* argv[]) {
  std::vector<std::string> args(argv, argv + argc);
  try {
    bool copy_attributes = false, move_file = false;
    for (const auto& parameter : args) {
      if (parameter == "-a") {
        copy_attributes = true;
        continue;
      }
      if (parameter == "-m") {
        move_file = true;
        continue;
      }
      if (copy_attributes && move_file) {
        std::filesystem::path exe_path = args[0];
        std::stringstream error;
        error << exe_path.filename().generic_string() << ": You can not use flags -m and -a simultaneously";
        throw std::runtime_error(error.str());
      }
    }
    int shift = 0;
    if (argc == 4) shift += 1;
    bool preserve_all = false;
    if (copy_attributes) preserve_all = true;
    std::string src_path = argv[1 + shift];
    std::string dst_path = argv[2 + shift];
    if (move_file) MoveFile(src_path, dst_path);
    else CopyFile(src_path, dst_path, preserve_all);
  } catch(...) {
    std::stringstream error;
    error << "Try " << args[0] << " --help for more information";
    std::throw_with_nested(std::runtime_error(error.str()));
  }
}