#include "Reader.h"

Reader::Reader(std::string filepath, DataFormat mode)
    : filepath_(filepath), mode_(mode) {}

Reader::~Reader() {
  if (file_.is_open()) {
    file_.close();
  }
}

void Reader::OpenFile() {
  if (file_.is_open()) {
    file_.close();  // Close the existing file.
  }

  file_.open(filepath_, std::ios_base::in | std::ios_base::binary);

  if (!file_.is_open()) {
    printf("Failed to open file %s\n", filepath_.c_str());
    throw FailedToOpenFileException(filepath_);
  }
}

std::vector<uint8_t> Reader::ReadFile(int num_bytes) {
  if (!FileExists(filepath_)) {
    throw FileDoesNotExistException(filepath_);
  }

  if (!file_.is_open()) {  // If the file is not open, open it
    OpenFile();
  }

  std::vector<uint8_t> buffer(num_bytes);
  file_.read((char*)buffer.data(), num_bytes);

  if (file_.fail() && !file_.eof()) {
    throw FailedToReadFromFileException(file_.getloc().name());
  }

  buffer.resize(
      file_
          .gcount());  // Resize the vector to the number of bytes actually read

  if (mode_ == DataFormat::NETASCII)  // if netascii, format the data
    return FormatToNETASCII(buffer);
  return buffer;  // else return the buffer as is
}