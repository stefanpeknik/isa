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

std::vector<uint8_t> Reader::ReadFile(unsigned int num_bytes) {
  if (!FileExists(filepath_)) {
    throw FileDoesNotExistException(filepath_);
  }

  if (!file_.is_open()) {  // If the file is not open, open it
    OpenFile();
  }

  // read overflow buffer first
  std::vector<uint8_t> bytes;
  if (overflow_buffer_.size() > num_bytes) {
    bytes = std::vector<uint8_t>(overflow_buffer_.begin(),
                                 overflow_buffer_.begin() + num_bytes);
    overflow_buffer_.erase(overflow_buffer_.begin(),
                           overflow_buffer_.begin() + num_bytes);
  } else {
    bytes =
        std::vector<uint8_t>(overflow_buffer_.begin(), overflow_buffer_.end());
    overflow_buffer_.clear();
  }

  if (bytes.size() < num_bytes) {
    // read the rest of the bytes from the file
    std::vector<uint8_t> file_bytes(num_bytes - bytes.size());
    file_.read((char *)file_bytes.data(), num_bytes - bytes.size());
    bytes.insert(bytes.end(), file_bytes.begin(), file_bytes.end());
  }

  if (mode_ == DataFormat::NETASCII) {  // if netascii, format the data
    bytes = FormatToNETASCII(bytes);
    if (bytes.size() > num_bytes) {
      // if the buffer is larger than the number of bytes requested, store the
      // overflow in the overflow buffer
      overflow_buffer_ =
          std::vector<uint8_t>(bytes.begin() + num_bytes, bytes.end());
      bytes.resize(num_bytes);
    } else {
      overflow_buffer_ = {};
    }
  }

  return bytes;
}
