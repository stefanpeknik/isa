#include "IOHandler.h"

IOHandler::IOHandler(IO io_type) : io_type(io_type) {}

IOHandler::~IOHandler() {
  if (file_.is_open()) {
    file_.close();
  }
}

void IOHandler::Write(std::vector<uint8_t> data) {
  switch (io_type) {
    case IO::FILE:
      WriteFile(data);
      break;
    case IO::STD:
      WriteStdout(data);
      break;
  }
}

std::vector<uint8_t> IOHandler::Read(int num_bytes) {
  switch (io_type) {
    case IO::FILE:
      return ReadFile(num_bytes);
    case IO::STD:
      return ReadStdin(num_bytes);
  }
  return {};  // to silence compiler warning
}

void IOHandler::WriteFile(std::vector<uint8_t> data) {
  if (!EnoughSpaceOnDisk(data.size())) {
    throw NotEnoughSpaceOnDiskException();
  }

  file_.write((char*)data.data(), data.size());

  if (file_.fail()) {
    throw FailedToWriteToFileException();
  }
}

std::vector<uint8_t> IOHandler::ReadFile(int num_bytes) {
  std::vector<uint8_t> buffer(num_bytes);
  file_.read((char*)buffer.data(), num_bytes);

  if (file_.fail()) {
    throw FailedToReadFromFileException();
  }

  buffer.resize(
      file_
          .gcount());  // Resize the vector to the number of bytes actually read
  return buffer;
}

void IOHandler::WriteStdout(std::vector<uint8_t> data) {
  std::cout.write((char*)data.data(), data.size());

  if (std::cout.fail()) {
    throw FailedToWriteToStdoutException();
  }
}

std::vector<uint8_t> IOHandler::ReadStdin(int num_bytes) {
  std::vector<uint8_t> buffer(num_bytes);
  std::cin.read((char*)buffer.data(), num_bytes);

  if (std::cin.fail()) {
    throw FailedToReadFromStdinException();
  }

  buffer.resize(
      std::cin
          .gcount());  // Resize the vector to the number of bytes actually read
  return buffer;
}

void IOHandler::OpenFile(std::string filepath, std::ios_base::openmode mode) {
  if (file_.is_open()) return;

  file_.open(filepath, mode);

  if (!file_.is_open()) {
    throw FailedToOpenFileException();
  }
}

bool IOHandler::EnoughSpaceOnDisk(uintmax_t num_bytes) {
  std::filesystem::path filepath = file_.getloc().name();
  std::filesystem::space_info space = std::filesystem::space(filepath);
  return space.available >= num_bytes;
}

bool IOHandler::FileExists(std::string filepath) {
  return std::filesystem::exists(filepath);
}

