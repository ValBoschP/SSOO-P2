/**
 * Universidad de La Laguna
 * Escuela Superior de Ingeniería y Tecnología
 * Grado en Ingeniería Informática
 * Asignatura: Sistemas Operativos
 * Curso: 2º
 * Práctica 2: SHELL PROJECT
 * @autor: Valeria Bosch Pérez (alu0101485287@ull.edu.es)
 * @date: 3 Jan 2023
 * @file: usages.h
 * @brief: usages functions
 * Referencias:
 * Enlaces de interés
 */
#ifndef USAGES_H
#define USAGES_H

#include <exception>

void Usage(const int argc, const char* argv[]);
void Program(const int argc, const char* argv[]);
void PrintException(const std::exception& error);

#endif