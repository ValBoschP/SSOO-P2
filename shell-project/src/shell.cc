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

/**
 * @brief Imprime los argumentos a la salida estándar
 * @param args Vector containing the command and its arguments.
 * 
 * @return Un entero indicando el éxito (0) o fallo (1) de la función.
 */
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

/**
 * @brief Cambia el directorio de trabajo actual.
 * @param args Vector de strings con los argumentos
 * @throw std::system_error Si la funcion falla da un system error.
 * 
 * @return Un entero indicando el éxito (0) o fallo (1) de la función.
 */
int Shell::CdCommand(const std::vector<std::string>& args) {
  try {
    // Verifica el número de argumentos correcto
    if (args.size() > 2) std::throw_with_nested(std::runtime_error("ERROR: Too many arguments!"));
    // Cambia el directorio actual
    if (args.size() == 1) {
      // Cambia al directorio home
      const char* home_directory;
      if ((home_directory = getenv("HOME")) == NULL) home_directory = getpwuid(getuid())->pw_dir;
      chdir(home_directory);
    } else {
      // Cambia al directorio especificado
      if (chdir(args[1].c_str()) != 0) {
        throw std::system_error(errno, std::system_category());
      }
    }
  } catch (const std::system_error& error) {
    std::throw_with_nested(std::runtime_error("ERROR: cd command failed!"));
    return 1;
  }
  return 0;
}

/**
 * @brief Copia o mueve un archivo de una ubicacion a otra
 * @param args Vector de strings con los argumentos
 * @throw std::system_error Si la funcion falla da un system error.
 * 
 * @return Un entero indicando el éxito (0) o fallo (1) de la función.
 */
int Shell::CpCommand(const std::vector<std::string>& args) {
  try {
    // Comprueba si existe los argumentos -a y -m del copyfile
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
    // Calcula los shift necesarioa para evitar los parametros si es necesario
    int shift = 0;
    if (args.size() == 4) shift += 1;
    // Determina si se debe preservar todos los atributos
    bool preserve_all = false;
    if (copy_attributes || move_file) preserve_all = true;
    // Obtenemos los caminos del origen y destino
    std::string src_path = args[1 + shift];
    std::string dst_path = args[2 + shift];
    // Llama a la función correspondiente para aplicarselo al archivo
    if (!move_file) {
      CopyFile(src_path, dst_path, preserve_all);
    } else {
      MvCommand(args);
    }
  } catch (const std::system_error& error) {
    std::throw_with_nested(std::runtime_error("ERROR: cp command failed!"));
    return 1;
  }
  return 0;
}

/**
 * @brief Mueve un archivo de una ubicación a otra o la renombra.
 * @param args Vector de strings con los argumentos
 * @throw std::system_error Si la funcion falla da un system error.
 * 
 * @return Un entero indicando el éxito (0) o fallo (1) de la función.
 */
int Shell::MvCommand(const std::vector<std::string>& args) {
  try {
    // Calcula los shift necesarioa para evitar los parametros si es necesario
    int shift = 0;
    if (args.size() == 4) shift += 1;
    // Obtenemos los caminos del origen y destino
    std::string src_path = args[1 + shift];
    std::string dst_path = args[2 + shift];
    // Llama a la función MoveFile para aplicarselo al archivo
    MoveFile(src_path, dst_path);
  } catch (const std::system_error& error) {
    std::throw_with_nested(std::runtime_error("ERROR: mv command failed!"));
    return 1;
  }
  return 0;
}

/**
 * @brief Ejecuta los comandos que se les pase (internos o externos)
 * @param commands Vector que contiene los comandos a evaluar
 * 
 * @return Un CommandResult indicando el éxito o fallo del comando y de la salida.
 */
CommandResult Shell::ExecuteCommand(const std::vector<std::string>& commands) {
  try {
    // Comprobar si los comandos recibidos son internos o no.
    bool incorrect_command = true;
    for (const auto& internal_command : internal_commands_) {
      if (commands[0] == internal_command) {
        incorrect_command = false;
        break;
      }
    }
    // Si es exit sale de la shell
    if (commands[0] == "exit") return CommandResult::Quit();
    // Si es cp -> COPYFILE
    else if (commands[0] == "echo") {
      return CommandResult(EchoCommand(commands), false);
    // Si es cd -> Comando cd
    } else if (commands[0] == "cd") {
      return CommandResult(CdCommand(commands), false);
    // Si es mv -> Comando mv
    } else if (commands[0] == "cp") {
      return CommandResult(CpCommand(commands), false);
    // Si es echo -> Comando echo
    } else if (commands[0] == "mv") {
      return CommandResult(MvCommand(commands), false);
      // En el caso de que no sea ningún comando interno, se ejecutará como comando externo.
    } else {
      return CommandResult(ExecuteProgram(commands, true), false);
    }
  } catch (const std::exception& error) {
    std::throw_with_nested(std::runtime_error("ERROR: Executing commands failed!"));
  }
  return CommandResult::Quit();
}

/**
 * @brief Ejecuta un programa externo
 * @param args Vector que contiene los comandos a evaluar
 * @param has_wait Bool que indica si esperar a que finalice el programa antes de volver a iniciar.
 * 
 * @return El estado de salida del programa si has_wait es verdadero, la identificación del proceso 
 *         secundario si has_wait es falso, un valor distinto de cero en caso de error.
 */
int Shell::ExecuteProgram(const std::vector<std::string>& args, bool has_wait = true) {
  try {
    // Crea un proceso hijo
    pid_t pid = fork();
    // Si falla lanza una excepcion
    if (pid < 0) {
      std::throw_with_nested(std::runtime_error("ERROR: Creating the process!"));
    }
    // Si estamos en el proceso padre
    if (pid > 0) {
      // Si tenemos que esperar a que el proceso hijo termine y retornar la salida
      if (has_wait) {
        int status;
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
        // Si no tenemos que esperar, se retorna el id del proceso hijo
      } else {
        return pid;
      }
      // Si estamos en el proceso hijo
    } else {
      // Convierte el vector de string a un vector de char* (strings)
      char** argv = new char*[args.size() + 1];
      for (int i = 0; i < args.size(); i++) {
        argv[i] = const_cast<char*>(args[i].c_str());
      }
      argv[args.size()] = nullptr;
      // Ejecuta el programa. Si la ejecución falla, se sale del programa.
      if (execvp(argv[0], argv) < 0) exit(EXIT_FAILURE);
    }
    return 0;
  } catch (const std::exception& error) {
    std::throw_with_nested(std::runtime_error("ERROR: Executing function failed!"));
  }
}

/**
 * @brief Imprime el error en la salida estándar
 * @param error El error a imprimir.
 */
void PrintError(const std::string& error) {
  std::cerr << "ERROR: " << error << '\n' << '\n';
  std::cerr.flush();
}

/**
 * @brief Ejecuta la shell
 */
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
      // Si la linea de entrada está vacía va a la siguiente iteacion del bucle
      if (line.empty()) continue;
      // Recorre cada uno de los comandos y los ejecuta
      for (const auto& cmd : commands) {
        // Se ejecuta el comando y obtenemos el resultado del comando
        auto [return_value, is_quit_requested] = ExecuteCommand(cmd);
        // Si se requiere el quit, se sale de la shell
        if (is_quit_requested) exit(EXIT_SUCCESS);
        std::cout << std::endl;
        // Actualiza el estado del ultimo comando
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