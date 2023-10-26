#include "Writer.h"

Writer::Writer(std::string filepath, DataFormat mode)
    : filepath_(filepath), mode_(mode) {}

Writer::~Writer() {
  if (file_.is_open()) {
    file_.close();
  }
}

void Writer::OpenFile() {
  if (file_.is_open()) {
    file_.close();  // Close the existing file.
  }

  file_.open(filepath_, std::ios_base::out | std::ios_base::binary);

  if (!file_.is_open()) {
    throw FailedToOpenFileException(filepath_);
  }
}

void Writer::WriteFile(std::vector<uint8_t> data) {
  if (!file_.is_open()) {  // If the file is not open, open it
    OpenFile();
  }

  file_.write((char*)data.data(), data.size());

  // force the buffer to be written to the file
  file_.flush();

  if (file_.fail()) {
    throw FailedToWriteToFileException(file_.getloc().name());
  }
}
