#include "FileHandler.h"

bool FileHandler::FileExists(std::string filepath) {
  return std::filesystem::exists(filepath);
}

void FileHandler::DeleteFile(std::string filepath) {
  std::filesystem::remove(filepath);
}

bool FileHandler::isFilePathUnderDirectory(std::string filePath,
                                           std::string directoryPath) {
  std::string relative = std::filesystem::relative(filePath, directoryPath);
  // Size check for a "." result.
  // If the path starts with "..", it's not a subdirectory.
  return relative.size() == 1 || (relative[0] != '.' && relative[1] != '.');
}

bool FileHandler::HasEnoughSpace(std::string path, uintmax_t requiredBytes) {
  struct statvfs stat;

  if (statvfs(path.c_str(), &stat) != 0) {
    // Error occurred while getting file system statistics
    return false;
  }

  // Calculate the available space in bytes
  uintmax_t availableBytes = stat.f_bavail * stat.f_frsize;

  // Check if there is enough space
  return availableBytes >= requiredBytes;
}
