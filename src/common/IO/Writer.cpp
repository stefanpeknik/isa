#include "Writer.h"

Writer::Writer(std::string filepath, DataFormat mode)
    : filepath_(filepath), mode_(mode) {}

Writer::~Writer() {
  if (file_.is_open()) {
    // write the tbw_later_buffer_ to the file
    if (tbw_later_buffer_.size() > 0) {
      file_.write((char *)tbw_later_buffer_.data(), tbw_later_buffer_.size());
    }
    file_.flush();
    file_.close();
  }
}

void Writer::OpenFile() {
  if (file_.is_open()) {
    file_.close(); // Close the existing file.
  }

  file_.open(filepath_, std::ios_base::out | std::ios_base::binary);

  if (!file_.is_open()) {
    throw FailedToOpenFileException(filepath_);
  }
}

std::string Writer::GetFilepath() { return filepath_; }

void Writer::WriteFile(std::vector<uint8_t> data) {
  if (!file_.is_open()) { // If the file is not open, open it
    OpenFile();
  }

  if (tbw_later_buffer_.size() > 0) {
    data.insert(data.begin(), tbw_later_buffer_.begin(),
                tbw_later_buffer_.end());
    tbw_later_buffer_.clear();
  }

  if (mode_ == DataFormat::NETASCII) {
    data = FormatFromNETASCII(data);
    // if the last char is '\r', dont write it yet and instead wait for next
    // write to check if '\n' or '\0' follows
    if (data[data.size() - 1] == '\r') {
      tbw_later_buffer_.push_back('\r');
      data.pop_back();
    }
  }

  file_.write((char *)data.data(), data.size());

  // force the buffer to be written to the file
  file_.flush();

  if (file_.fail()) {
    throw FailedToWriteToFileException(file_.getloc().name());
  }
}
