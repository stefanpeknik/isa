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
    file_.close(); // Close the existing file.
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

  if (!file_.is_open()) { // If the file is not open, open it
    OpenFile();
  }

  // the final buffer to be returned
  std::vector<uint8_t> buffer;

  if (mode_ == DataFormat::NETASCII) { // if netascii, format the data
    if (overflow_buffer_.size() >
        0) { // if there is data in the overflow buffer
      if (overflow_buffer_.size() >=
          num_bytes) { // if overflow buffer is larger than the number of bytes
                       // requested
        buffer = std::vector<uint8_t>(overflow_buffer_.begin(),
                                      overflow_buffer_.begin() + num_bytes);
        overflow_buffer_.erase(
            overflow_buffer_.begin(),
            overflow_buffer_.begin() +
                num_bytes); // erase the read data from the overflow buffer
        auto last_index = buffer.size() - 1;
        // Check last index to see if there is a '\r' or '\n' character
        while (buffer[last_index] == '\r' || buffer[last_index] == '\n') {
          if (overflow_buffer_.size() >
              0) { // if there is data in the overflow buffer
            buffer.push_back(overflow_buffer_[0]);
            overflow_buffer_.erase(overflow_buffer_.begin());
          } else { // else get data from file
            uint8_t data;
            file_.read(reinterpret_cast<char *>(&data), 1);
            if (file_.fail() && !file_.eof()) {
              throw FailedToReadFromFileException(file_.getloc().name());
            }
            buffer.push_back(data);
            if (file_.eof()) { // if end of file, break
              break;
            }
          }
          last_index = buffer.size() - 1; // update last index
          // keep reading until there is no '\r' or '\n' character at the end
        }
      } else { // overflow buffer is smaller than the number of bytes requested
        buffer = overflow_buffer_;
        overflow_buffer_ = {};
      }
    }
    if (buffer.size() < num_bytes) { // if buffer is smaller than the number of
                                     // bytes requested
      auto to_be_read = num_bytes - buffer.size();
      std::vector<uint8_t> data(to_be_read);
      file_.read(reinterpret_cast<char *>(data.data()), to_be_read);
      if (file_.fail() && !file_.eof()) {
        throw FailedToReadFromFileException(file_.getloc().name());
      }
      data.resize(file_.gcount()); // resize data to the number of bytes read
      buffer.insert(buffer.end(), data.begin(), data.end());
      if (buffer.size() == num_bytes &&
          file_.eof() == false) { // if buffer is equal to the number of
                                  // bytes requested AND end of file is not
                                  // reached
        while (buffer[buffer.size() - 1] == '\r' ||
               buffer[buffer.size() - 1] == '\n') { // check last index for
                                                    // '\r' or '\n' character
          uint8_t data;
          file_.read(reinterpret_cast<char *>(&data), 1);
          if (file_.fail() && !file_.eof()) {
            throw FailedToReadFromFileException(file_.getloc().name());
          }
          buffer.push_back(data);
          if (file_.eof()) { // if end of file, break
            break;
          }
        }
      }
    }

    buffer = FormatToNETASCII(buffer);
    // if buffer len < num bytes to be read => we found EOF and we need to
    // ensure that the last line
    if (buffer.size() < num_bytes || // if buffer is smaller than the number of
                                     // bytes requested => we found EOF
        // if buffer is larger than the number of bytes requested and the last
        // two characters are not '\r\n'
        (buffer.size() >= 2 && (buffer[buffer.size() - 2] != '\r' ||
                                buffer[buffer.size() - 1] != '\n'))) {
      buffer.push_back('\r');
      buffer.push_back('\n');
    }
    if (buffer.size() > num_bytes) { // if buffer is larger than the number of
                                     // bytes requested
      overflow_buffer_ = std::vector<uint8_t>(
          buffer.begin() + num_bytes,
          buffer.end());        // put the extra data in the overflow buffer
      buffer.resize(num_bytes); // resize buffer to the number of bytes
                                // requested
    }
  } else { // flat read data from file
    std::vector<uint8_t> data(num_bytes);
    file_.read(reinterpret_cast<char *>(data.data()), num_bytes);
    if (file_.fail() && !file_.eof()) {
      throw FailedToReadFromFileException(file_.getloc().name());
    }
    data.resize(file_.gcount()); // resize data to the number of bytes read
    buffer = data;
  }

  return buffer; // return the buffer
}

std::string Reader::GetFilepath() { return filepath_; }
