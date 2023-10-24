#include "FileHandler.h"

bool FileHandler::EnoughSpaceOnDisk(std::string root_dir, uintmax_t num_bytes) {
  // TODO: Implement this
  return true;
}

bool FileHandler::FileExists(std::string filepath) {
  return std::filesystem::exists(filepath);
}

void FileHandler::DeleteFile(std::string filepath) {
  std::filesystem::remove(filepath);
}
