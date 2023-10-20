#ifndef IOHandler_h
#define IOHandler_h

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

class IOHandler {
 public:
  enum class IO { FILE, STD };
  IOHandler(IO io_type);
  ~IOHandler();

  IO io_type;

  void Write(std::vector<uint8_t> data);
  std::vector<uint8_t> Read(int num_bytes);

  void WriteFile(std::vector<uint8_t> data);
  std::vector<uint8_t> ReadFile(int num_bytes);

  void WriteStdout(std::vector<uint8_t> data);
  std::vector<uint8_t> ReadStdin(int num_bytes);

  void OpenFile(std::string filepath, std::ios_base::openmode mode);

  bool EnoughSpaceOnDisk(uintmax_t num_bytes);

 private:
  std::fstream file_;
};

class IOHandlerException : public std::exception {
 public:
  IOHandlerException(const std::string& message) : message(message) {}

  const char* what() const noexcept override { return message.c_str(); }

 private:
  std::string message;
};

class FailedToOpenFileException : public IOHandlerException {
 public:
  FailedToOpenFileException()
      : IOHandlerException("Error: Failed to open file") {}
};

class FailedToWriteToFileException : public IOHandlerException {
 public:
  FailedToWriteToFileException()
      : IOHandlerException("Error: Failed to write to file") {}
};

class FailedToReadFromFileException : public IOHandlerException {
 public:
  FailedToReadFromFileException()
      : IOHandlerException("Error: Failed to read from file") {}
};

class FailedToCloseFileException : public IOHandlerException {
 public:
  FailedToCloseFileException()
      : IOHandlerException("Error: Failed to close file") {}
};

class FailedToReadFromStdinException : public IOHandlerException {
 public:
  FailedToReadFromStdinException()
      : IOHandlerException("Error: Failed to read from stdin") {}
};

class FailedToWriteToStdoutException : public IOHandlerException {
 public:
  FailedToWriteToStdoutException()
      : IOHandlerException("Error: Failed to write to stdout") {}
};

class NotEnoughSpaceOnDiskException : public IOHandlerException {
 public:
  NotEnoughSpaceOnDiskException()
      : IOHandlerException("Error: Not enough space on disk") {}
};

#endif  // IOHandler_h
