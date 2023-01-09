/**
 * Universidad de La Laguna
 * Escuela Superior de Ingeniería y Tecnología
 * Grado en Ingeniería Informática
 * Asignatura: Sistemas Operativos
 * Curso: 2º
 * Práctica 2: SHELL PROJECT
 * @autor: Valeria Bosch Pérez (alu0101485287@ull.edu.es)
 * @date: 3 Jan 2023
 * @file: shell.h
 * @brief: shell class
 * Referencias:
 * Enlaces de interés
 */
#ifndef SHELL_H
#define SHELL_H

#include <sys/types.h>
#include <string>
#include <map>
#include <functional>
#include <vector>

#include "shell_system.h"

/**
 * @brief Clase que representa una SHELL
 */
class Shell {
 public:
  // Constructor
  Shell(const pid_t& procces_id) : procces_id_(procces_id) {}
  Shell() : procces_id_(0) {}

  // Getter
  inline const std::vector<std::string>& GetInternalCommands() const { return internal_commands_; }

  // Comandos internos de la shell
  int EchoCommand(const std::vector<std::string>& args);
  int CdCommand(const std::vector<std::string>& args);
  int CpCommand(const std::vector<std::string>& args);
  int MvCommand(const std::vector<std::string>& args);

  // Comandos internos y externos
  CommandResult ExecuteCommand(const std::vector<std::string>& commands);
  int ExecuteProgram(const std::vector<std::string>& args, bool has_wait);

  // Ejecutar la shell
  void Run();

 private:
  pid_t procces_id_;
  std::vector<std::string> internal_commands_ = { "cd", "echo", "cp", "mv", "exit" };
};

#endif