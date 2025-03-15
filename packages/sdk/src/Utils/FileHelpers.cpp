#include <cassert>

#include <IRacingSDK/Utils/FileHelpers.h>
#include <IRacingSDK/Utils/StringHelpers.h>
#include <IRacingSDK/Utils/UnicodeHelpers.h>
#include <gsl/util>

namespace IRacingSDK::Utils {
  namespace fs = std::filesystem;


  std::vector<fs::path> ListAllFiles(const std::vector<fs::path> &paths, bool recursive, const std::string &ext) {
    std::vector<fs::path> files{};

    for (auto &path : paths) {
      auto iterateFiles = [&](auto it) {
        for (auto &fileEntry : it) {
          auto &file = fileEntry.path();
          if (ext.empty() || file.string().ends_with(ext)) {
            files.push_back(file);
          }
        }
      };

      if (recursive) {
        iterateFiles(fs::recursive_directory_iterator(path));
      } else {
        iterateFiles(fs::directory_iterator(path));
      }
    }
    return files;
  }


  /**
   * @brief Fully reads data from a file stream into a buffer.
   *
   * This function reads data from the given file stream into the provided buffer until either the specified
   * `count` is reached or end-of-file is encountered. The total number of bytes read is tracked by the `totalRead`
   * variable. If the number of bytes read is zero at any point, the function stops and returns `false`.
   *
   * @param buffer Pointer to the destination buffer to store the read data.
   * @param size Size in bytes of each element to read from the file stream.
   * @param count Maximum number of elements to read from the file stream.
   * @param stream Pointer to the file stream to read data from.
   *
   * @return `true` if the requested number of elements were read successfully, `false` otherwise.
   */
  bool FileReadDataFully(void *buffer, std::size_t size, std::size_t count, std::FILE *stream) {
    std::size_t totalRead = 0, read;
    while (totalRead != count && (read = std::fread(buffer, size, count, stream)) > 0) {
      totalRead += read;
      if (read == 0) {
        return false;
      }
    }

    return true;
  }

  /**
   * @brief Checks if a file has a specific extension.
   *
   * The function checks whether the given file has the specified extension.
   * It compares the extension of the file with the provided extension.
   *
   * @param file The file to check.
   * @param ext The extension to compare with.
   *
   * @return true if the file has the specified extension, false otherwise.
   */
  bool HasFileExtension(const std::filesystem::path &file, const std::string &ext) {
    return HasFileExtension(file, std::vector{ext});
  }

  /**
   * @brief Checks if a file has one of the specified extensions.
   *
   * This function checks if the given file has any of the extensions provided in the list.
   * The comparison is case-insensitive.
   *
   * @param file The path of the file to check.
   * @param exts A vector of strings representing the file extensions to match against.
   *
   * @return true if the file's extension matches any of the specified extensions, false otherwise.
   */
  bool HasFileExtension(const std::filesystem::path &file, const std::vector<std::string> &exts) {
    auto filename = Strings::toLowerCase(file.filename().string());
    return std::ranges::any_of(
      exts,
      [&filename](auto &ext) {
        return filename.ends_with(Strings::toLowerCase(ext));
      });
  }

  /**
   * @brief Reads the contents of a file into a vector of unsigned characters.
   *
   * @param path The path to the file to be read.
   * @return An Expected object containing the vector of unsigned characters if successful, or an error message if unsuccessful.
   */
  Expected<std::vector<unsigned char>> ReadFile(const fs::path &path) {
    size_t size;
    if (!exists(path) || (size = file_size(path)) == 0) {
      return std::unexpected(GeneralError(ErrorCode::General, std::format("File does not exist or is empty ({})", path.string())));
    }

    std::vector<unsigned char> data(size);
    auto buf = data.data();
    auto pathStr = ToUtf8(path);
    auto file = std::fopen(pathStr.c_str(), "rb");
    if (!file) {
      return std::unexpected(GeneralError(ErrorCode::General, std::format("Unable to open ({}): {}", path.string(), std::string(std::strerror(errno)))));
    }

    auto cleanup = FILE_RESOURCE_DISPOSER(file);

    {
      size_t readTotal = 0;

      while (readTotal < size) {
        size_t read = std::fread(buf + readTotal, 1, size - readTotal, file);
        if (!read) {
          break;
        }
        readTotal += read;
      }

      if (readTotal != size)
        return std::unexpected(GeneralError(ErrorCode::General, std::format("Did not read correct number of bytes ({} != {})", readTotal, size)));
    }


    return data;
  }

  /**
   * @brief Reads the contents of a text file.
   *
   * Given a file path, this function reads the contents of the text file
   * and returns it as a string.
   *
   * @param path The path to the text file.
   *
   * @return An Expected object containing the contents of the text file
   *         as a string. If the file cannot be read, an unexpected object
   *         is returned with the appropriate error message.
   */
  Expected<std::string> ReadTextFile(const std::filesystem::path &path) {
    auto res = ReadFile(path);
    if (!res) {
      return std::unexpected(res.error());
    }

    auto &txtData = res.value();
    return std::string(reinterpret_cast<char *>(txtData.data()), txtData.size());
  }


  /**
   * Writes the contents of a buffer to a file.
   *
   * @param path The path to the file.
   * @param buf The buffer that contains the data to write.
   * @param size The size of the data to write.
   * @return The number of bytes written to the file on success, or an unexpected error on failure.
   */
  Expected<std::size_t> WriteFile(const std::filesystem::path &path, const unsigned char *buf, size_t size) {
    auto dir = path.parent_path();
    if (!exists(dir) || !is_directory(dir)) {
      return std::unexpected(GeneralError(ErrorCode::General, std::format("File does not exist or is empty ({})", path.string())));
    }

    auto pathStr = ToUtf8(path);
    auto file = std::fopen(pathStr.c_str(), "wb");
    if (!file) {
      return std::unexpected(GeneralError(ErrorCode::General, std::format("Unable to open ({}): {}", path.string(), std::string(std::strerror(errno)))));
    }

    auto cleanup = FILE_RESOURCE_DISPOSER(file);
    size_t writeTotal = 0;

    while (writeTotal < size) {
      size_t writeCount = std::fwrite(buf + writeTotal, 1, size - writeTotal, file);
      if (!writeCount) {
        break;
      }
      writeTotal += writeCount;
    }

    if (writeTotal != size)
      return std::unexpected(GeneralError(ErrorCode::General, std::format("Did not write correct number of bytes ({} != {})", writeTotal, size)));


    return writeTotal;
  }


  /**
   * @brief Writes the provided data to a file at the given path.
   *
   * @param path The path to the file to be written.
   * @param data The data to be written to the file.
   *
   * @return An Expected object containing the result of the write operation. On success, it contains the number of bytes written to the file. On failure, it contains an error code.
   */
  Expected<std::size_t> WriteFile(const std::filesystem::path &path, const std::vector<unsigned char> &data) {
    return WriteFile(path, data.data(), data.size());
  }

  /**
   * @brief Writes text to a file.
   *
   * This function writes the specified text to the file located at the specified path.
   *
   * @param path The path to the file.
   * @param txt The text to write.
   * @return An Expected object containing the number of bytes written on success, or an error code on failure.
   */
  Expected<std::size_t> WriteTextFile(const std::filesystem::path &path, const std::string &txt) {
    return WriteFile(path, reinterpret_cast<const uint8_t *>(txt.c_str()), txt.length());
  }

  /**
   * @brief Searches for an executable in the system's PATH environment variable.
   *
   * This function attempts to locate the specified executable by searching through the directories
   * listed in the system's PATH. If the executable is found, a string containing the full path is returned.
   * If it is not found or an error occurs, an unexpected error result is returned.
   *
   * @param exeName Name of the executable to search for in the system's PATH.
   *
   * @return On success, returns an `Expected` containing the full path to the executable.
   *         On failure, returns an unexpected error containing an appropriate error code and message.
   */
  Expected<std::string> FindExeInPath(const std::string &exeName) {
    char buffer[MAX_PATH];

    // Search for the executable in the system's PATH
    DWORD result = SearchPathA(nullptr, exeName.c_str(), nullptr, MAX_PATH, buffer, nullptr);

    if (result > 0 && result <= MAX_PATH) {
      return std::string(buffer, result);
    }

    if (GetLastError() == ERROR_FILE_NOT_FOUND) {
      return std::unexpected(GeneralError(ErrorCode::NotFound, fmt::format("Executable '{}', NOT_FOUND in PATH", exeName)));
    }
    return std::unexpected(GeneralError(ErrorCode::General, fmt::format("Error searching for executable '{}': {}", exeName, GetLastError())));
  }

  Expected<std::string> FindExeInPath(const std::filesystem::path &exePath) {
    return FindExeInPath(exePath.string());
  }

  FileLock::FileLock(const std::string &filename) :
      filename_(filename) {
    // Open or create the file to lock
    file_ = CreateFileA(
      filename.c_str(),
      GENERIC_READ | GENERIC_WRITE,
      0, // Do not allow sharing the file
      nullptr,
      OPEN_ALWAYS,
      FILE_ATTRIBUTE_NORMAL,
      nullptr);

    if (file_ == INVALID_HANDLE_VALUE) {
      throw std::runtime_error("Failed to open file for locking: " + filename);
    }
  }

  FileLock::~FileLock() {
    unlock(); // Ensure file is unlocked
    if (file_ != INVALID_HANDLE_VALUE) {
      CloseHandle(file_);
    }
  }

  void FileLock::lock() {
    if (locked_) {
      return; // Already locked
    }

    OVERLAPPED overlapped = {0};
    if (!LockFileEx(
          file_,
          LOCKFILE_EXCLUSIVE_LOCK,
          0,
          MAXDWORD,
          MAXDWORD,
          &overlapped)) {
      throw std::runtime_error("Failed to lock file: " + filename_);
    }

    locked_ = true;
  }

  void FileLock::unlock() {
    if (!locked_) {
      return; // Not locked
    }

    OVERLAPPED overlapped = {0};
    if (!UnlockFileEx(file_, 0, MAXDWORD, MAXDWORD, &overlapped)) {
      throw std::runtime_error("Failed to unlock file: " + filename_);
    }

    locked_ = false;
  }

  std::string FileLock::filename() const {
    return filename_;
  }


} // namespace IRacingSDK::Utils
