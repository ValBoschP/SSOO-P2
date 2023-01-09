#include "shell-system.h"
#include "scope_exit.h"

/**
 * @brief Lee un archivo y devuelve su contenido en un vector de bytes.
 * @param fd Descriptor del archivo.
 * @throw std::system_error Si se produce un error al leer el archivo.
 * @throw std::runtime_error Si se produce un error al leer el archivo.
 * 
 * @return Vector de bytes con el contenido del archivo.
 */
std::vector<uint8_t> ReadFile(const int fd) {
  try {
    std::vector<uint8_t> buffer(16ul * 1024 * 1024);
    ssize_t bytes_read = read(fd, buffer.data(), buffer.size());
    if (bytes_read < 0) throw std::system_error(errno, std::system_category());
    buffer.resize(bytes_read);
    return buffer;
  } catch (const std::exception& error) {
    std::throw_with_nested(std::runtime_error("Error reading file"));
  }
}

/**
 * @brief Escribe un vector de bytes en un archivo.
 * @param fd Descriptor del archivo.
 * @param buffer Vector de bytes a escribir en el archivo.
 * @throw std::runtime_error Si se produce un error al escribir el archivo.
 * 
 * @return Vector de bytes escrito en el archivo.
 */
std::vector<uint8_t> WriteFile(int fd, std::vector<uint8_t> buffer) {
  try {
    write(fd, buffer.data(), buffer.size());
    return buffer;
  } catch (const std::exception& error) {
    std::throw_with_nested(std::runtime_error("Error writing the file"));
  }
}

/**
 * @brief Divide una cadena de entrada en un vector de subcadenas, utilizando separadores y tokens especificados.
 * @param input_string Cadena de entrada.
 * @param separators Vector de caracteres que se usan como separadores.
 * @param tokens Vector de caracteres que se usan como tokens.
 *
 * @return Vector de subcadenas resultantes de dividir la cadena de entrada.
 */
std::vector<std::string> Split(const std::string& input_string, std::vector<char> separators, std::vector<char> tokens) {
  std::vector<std::string> characters;
  std::stringstream character;
  for (char symbol : input_string) {
    bool is_separator = false;
    bool is_token = false;
    for (const auto& separator : separators) {
      if (symbol == separator) {
        is_separator = true;
        break;
      }
    }
    for (const auto& token : tokens) {
      if (symbol == token) {
        is_token = true;
        break;
      }
    }
    if (is_token) {
      if (character.str().size() != 0) {
        characters.emplace_back(character.str());
        character.str(std::string());
      }
      character << symbol;
      characters.emplace_back(character.str());
      character.str(std::string());
      continue;
    } else if (!is_separator) {
      character << symbol;
      continue;
    }
    if (character.str().size() == 0) continue;
    characters.emplace_back(character.str());
    character.str(std::string());
  }
  characters.emplace_back(character.str());
  return characters;
}

/**
 * @brief Divide una cadena de entrada en un vector de subcadenas, utilizando espacios en blanco como separadores.
 * @param input_string Cadena de entrada.
 *
 * @return Vector de subcadenas resultantes de dividir la cadena de entrada.
 */
std::vector<std::string> SplitSpaces(const std::string& input_string) {
  std::vector<std::string> characters;
  std::stringstream character;
  for (char symbol : input_string) {
    if (symbol != ' ') {
      character << symbol;
      continue;
    }
    characters.emplace_back(character.str());
    character.str(std::string());
  }
  characters.emplace_back(character.str());
  return characters;
}

/**
 * @brief Divide una línea en un vector de comandos, utilizando el carácter '|' 
 *        como separador de pipes y el carácter ';' como separador de sentencias múltiples.
 * @param line Línea de entrada.
 *
 * @return Vector de vectores de subcadenas, donde cada subvector contiene un comando y sus argumentos.
 */
std::vector<std::vector<std::string>> ParseLine(const std::string& line) {
  std::vector<std::vector<std::string>> result;
  auto commands = Split(line, std::vector<char>{'|', ';'}, std::vector<char>());
  for (const auto& command : commands) {
    auto token = SplitSpaces(line);
    result.emplace_back(token);
  }
  return result;  
}

/**
 * @brief Copia un archivo de una ruta de origen a una ruta de destino.
 * @param source_path Ruta del archivo de origen.
 * @param destination_path Ruta del archivo de destino.
 * @param preserve_all Indica si se deben preservar todas las propiedades del archivo de origen (permisos, propietario, fechas de acceso y modificación).
 * @throw std::system_error Si se produce un error al abrir o cerrar el archivo de origen o destino.
 * @throw std::runtime_error Si se produce un error al copiar el archivo.
 */
void CopyFile(const std::string& source_path, const std::string& destination_path, bool preserve_all) {
  try {
    // Obtiene el stat del source_path
    struct stat source_path_stat{};
    if (stat(source_path.c_str(), &source_path_stat) == -1  || !S_ISREG(source_path_stat.st_mode)) {
      // Da una excepción si el camino origen no existe
      std::throw_with_nested(std::runtime_error("ERROR: Source path does not exist or source file is not a regular file!"));
    }
    // Obtiene el camino de destino y el nombre de directorio de destino
    std::string destination_path_copy = destination_path;
    char* c_destination_path = const_cast<char*>(destination_path.c_str());
    std::string dst_dir_name = dirname(c_destination_path);
    struct stat dst_dir_name_stat{};
    // Comprueba si el directorio de destino existe
    if (stat(dst_dir_name.c_str(), &dst_dir_name_stat) == -1) {
      std::throw_with_nested(std::runtime_error("ERROR: Destination path does not exist!"));
    }
    // Comprueba si el camino de destino es un directorio
    struct stat destination_path_stat{};
    destination_path_copy = destination_path;
    stat(destination_path_copy.c_str(), &destination_path_stat);
    if (S_ISDIR(destination_path_stat.st_mode)) {
      std::string source_path_copy = source_path;
      char* c_source_path = const_cast<char*>(source_path.c_str());
      std::string src_base_name = basename(c_source_path);
      destination_path_copy += "/" + src_base_name;
    }
    // Comprueba si el source path y el destination path son iguales
    if (source_path_stat.st_dev == destination_path_stat.st_dev && source_path_stat.st_ino == destination_path_stat.st_ino) {
      std::stringstream error;
      error << "'" << source_path << "' is the same file as '" << destination_path << "'";
      std::throw_with_nested(std::runtime_error(error.str()));
    }

    int source_fd = open(source_path.c_str(), O_RDONLY);
    auto close_src = ScopeExit([source_fd]{
      close(source_fd);
    });
    if (source_fd < 0) {
      throw std::system_error(errno, std::system_category());
    }

    int destination_fd = open(destination_path_copy.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
    auto close_dst = ScopeExit([destination_fd]{
      close(destination_fd);
    });
    if (destination_fd < 0) {
      throw std::system_error(errno, std::system_category());
    }

    while (true) {
      std::vector<uint8_t> buffer = ReadFile(source_fd);
      if (buffer.empty()) break;
      WriteFile(destination_fd, buffer);
    }

    if (preserve_all) {
      chmod(destination_path_copy.c_str(), source_path_stat.st_mode);
      chown(destination_path_copy.c_str(), source_path_stat.st_uid, source_path_stat.st_gid);
      struct utimbuf times{};
      times.actime = source_path_stat.st_atim.tv_sec;
      times.modtime = source_path_stat.st_mtim.tv_sec;
      utime(destination_path_copy.c_str(), &times);
    }
  } catch (const std::exception& error) {
    std::throw_with_nested(std::runtime_error("ERROR: Copying the file!"));
  }
}

/**
 * @brief Mueve un archivo de una ruta de origen a una ruta de destino.
 * @param source_path Ruta del archivo de origen.
 * @param destination_path Ruta del archivo de destino.
 * @throw std::runtime_error Si se produce un error al mover el archivo.
 */
void MoveFile(const std::string& source_path, const std::string& destination_path) {
  try {
    struct stat source_path_stat{};
    if (stat(source_path.c_str(), &source_path_stat) == -1  || !S_ISREG(source_path_stat.st_mode)) {
     std::throw_with_nested(std::runtime_error("ERROR: Source path does not exist or source file is not a regular file!"));
    }

    std::string destination_path_copy = destination_path;
    char* c_destination_path = const_cast<char*>(destination_path.c_str());
    std::string dst_dir_name = dirname(c_destination_path);
    struct stat dst_dir_name_stat{};
    if (stat(dst_dir_name.c_str(), &dst_dir_name_stat) == -1) {
      std::throw_with_nested(std::runtime_error("ERROR: Destination path does not exist!"));
    }

    struct stat destination_path_stat{};
    destination_path_copy = destination_path;
    stat(destination_path_copy.c_str(), &destination_path_stat);
    if (S_ISDIR(destination_path_stat.st_mode)) {
      std::string source_path_copy = source_path;
      char* c_source_path = const_cast<char*>(source_path.c_str());
      std::string src_base_name = basename(c_source_path);
      destination_path_copy += "/" + src_base_name;
    }

    if (source_path_stat.st_dev == destination_path_stat.st_dev && source_path_stat.st_ino == destination_path_stat.st_ino) {
      if (rename(source_path.c_str(), destination_path.c_str()) == -1) {
        throw std::runtime_error("Error renaming " + source_path + " to " + destination_path);
      }
      rename(source_path.c_str(), destination_path.c_str());
    } else {
      CopyFile(source_path, destination_path, 1);
      unlink(source_path.c_str());
    }
  } catch (const std::exception& error) {
    std::throw_with_nested(std::runtime_error("ERROR: Moving the file!"));
  }
}

/**
 * @brief Imprime una cadena en la salida estándar.
 * @param output_string Cadena de salida.
 * @throw std::system_error Si se produce un error al escribir en la salida estándar.
 */
void PrintLine(const std::string& output_string) {
  int bytes_written = write(STDOUT_FILENO, output_string.c_str(), output_string.size());
  if (bytes_written == -1) throw std::system_error(errno, std::system_category());
}

/**
 * @brief Imprime el prompt de la shell en la salida estándar.
 * @param last_command_status Estado del último comando ejecutado. Si es cero, el prompt se muestra en verde; en caso contrario, se muestra en rojo.
 */
void PrintPrompt(int last_command_status) {
  if (!isatty(STDIN_FILENO)) return;
  char* username = getpwuid(getuid())->pw_name;
  char* hostname = new char[1024];
  char* current_work_directory = new char[1024];
  gethostname(hostname, 1024);
  getcwd(current_work_directory, 1024);
  std::stringstream prompt;
  std::string work_directory = current_work_directory;
  std::string home = getpwuid(getuid())->pw_dir;
  std::string root = "~";

  size_t pos = work_directory.find(home);
  if (pos != std::string::npos) {
    work_directory.replace(pos, home.length(), root);
  }
  std::string arrow = last_command_status == 0 ? "> " : "< ";
  prompt << username << "@" << hostname << work_directory << last_command_status << arrow << std::endl;
  PrintLine(prompt.str());
}

/**
 * @brief Lee una línea de un descriptor de archivo.
 * @param fd Descriptor de archivo.
 * @throw std::system_error Si se produce un error al leer del descriptor de archivo.
 * @throw std::runtime_error Si se produce un error al procesar la línea leída.
 * 
 * @return Cadena con la línea leída.
 */
std::string ReadLine(int fd) {
  std::vector<uint8_t> pending_input;
  std::regex only_spaces("^[ ]*$");
  try {
    while (true) {
      std::string line;
      for (const auto& character : pending_input) {
        if (character == '\n') {
          if (std::regex_match(line, only_spaces)) return "";
          else return line;
        } 
        line.push_back(character);
      }
      std::vector<uint8_t> buffer(1024ul);
      int bytes_read = read(fd, buffer.data(), buffer.size());
      if (bytes_read < 0) throw std::system_error(errno, std::system_category());
      if (buffer.empty()) {
        if (!pending_input.empty()) {
          for (const auto& character : pending_input) {
            line.push_back(character);
          }
        }
        pending_input.clear();
        return line;
      }
      pending_input.insert(pending_input.end(), std::make_move_iterator(buffer.begin()), std::make_move_iterator(buffer.end()));
    }
  } catch (const std::system_error& error) {
    std::throw_with_nested(std::runtime_error("ERROR: Reading the input"));
  }
}

