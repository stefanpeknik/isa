#include "StdHandler.h"

StdHandler::StdHandler(DataFormat mode) : mode_(mode) {}

void StdHandler::WriteBytesToStdout(std::vector<uint8_t> data) {
  const char *dataPtr = reinterpret_cast<const char *>(data.data());
  size_t size = data.size();
  std::cout.write(dataPtr, size);
}

std::vector<uint8_t> StdHandler::ReadBytesFromStdin(size_t size) {
  std::vector<uint8_t> buffer(size);
  char *bufferPtr = reinterpret_cast<char *>(buffer.data());
  std::cin.read(bufferPtr, size);

  if (mode_ == DataFormat::NETASCII)  // if netascii, format the data
    return FormatToNETASCII(buffer);
  return buffer;  // else return the buffer as is
}
