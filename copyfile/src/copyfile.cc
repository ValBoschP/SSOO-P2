/**
 * Universidad de La Laguna
 * Escuela Superior de Ingeniería y Tecnología
 * Grado en Ingeniería Informática
 * Asignatura: Sistemas Operativos
 * Curso: 2º
 * Práctica 2: SHELL PROJECT
 * @autor: Valeria Bosch Pérez (alu0101485287@ull.edu.es)
 * @date: 3 Jan 2023
 * @file: shell_system.cc
 * @brief: shell system functions
 * Referencias:
 * Enlaces de interés
 */

#include "copyfile.h"
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
    std::throw_with_nested(std::runtime_error("ERROR: Reading file failed!"));
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
    std::throw_with_nested(std::runtime_error("ERROR: Writing file failed!"));
  }
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
        throw std::runtime_error("ERROR: Renaming " + source_path + " to " + destination_path);
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