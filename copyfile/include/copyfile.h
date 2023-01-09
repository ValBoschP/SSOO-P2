/**
 * Universidad de La Laguna
 * Escuela Superior de Ingeniería y Tecnología
 * Grado en Ingeniería Informática
 * Asignatura: Sistemas Operativos
 * Curso: 2º
 * Práctica 2: SHELL PROJECT
 * @autor: Valeria Bosch Pérez (alu0101485287@ull.edu.es)
 * @date: 3 Jan 2023
 * @file: shell_system.h
 * @brief: shell system functions
 * Referencias:
 * Enlaces de interés
 */
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
#include <string>

std::vector<uint8_t> ReadFile(const int fd);
std::vector<uint8_t> WriteFile(int fd, std::vector<uint8_t> buffer);

// COPY AND MOVE FUNCTIONS
void CopyFile(const std::string& src_path, const std::string& dst_path, bool preserve_all);
void MoveFile(const std::string& src_path, const std::string& dst_path);

#endif