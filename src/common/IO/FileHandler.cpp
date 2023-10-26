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
