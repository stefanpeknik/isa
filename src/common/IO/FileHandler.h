#ifndef FileHandler_h
#define FileHandler_h

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#include "IOCommon.h"

class FileHandler {
 public:
  virtual void OpenFile() = 0;

  static bool EnoughSpaceOnDisk(std::string root_dir, uintmax_t num_bytes);
  static bool FileExists(std::string filepath);
  static void DeleteFile(std::string filepath);
};

class FileHandlerException : public std::exception {
 public:
  FileHandlerException(const std::string& message) : message(message) {}

  const char* what() const noexcept override { return message.c_str(); }

 private:
  std::string message;
};

class FailedToOpenFileException : public FileHandlerException {
 public:
  FailedToOpenFileException(const std::string& filename)
      : FileHandlerException("Error: Failed to open file " + filename) {}
};

class FailedToWriteToFileException : public FileHandlerException {
 public:
  FailedToWriteToFileException(const std::string& filename)
      : FileHandlerException("Error: Failed to write to file " + filename) {}
};

class FailedToReadFromFileException : public FileHandlerException {
 public:
  FailedToReadFromFileException(const std::string& filename)
      : FileHandlerException("Error: Failed to read from file " + filename) {}
};

class FileDoesNotExistException : public FileHandlerException {
 public:
  FileDoesNotExistException(const std::string& filename)
      : FileHandlerException("Error: File " + filename + " does not exist") {}
};

class DataInWrongFormatException : public FileHandlerException {
 public:
  DataInWrongFormatException()
      : FileHandlerException("Error: Data in wrong format") {}
};

class NotEnoughSpaceOnDiskException : public FileHandlerException {
 public:
  NotEnoughSpaceOnDiskException()
      : FileHandlerException("Error: Not enough space on disk") {}
};

#endif  // FileHandler_h
