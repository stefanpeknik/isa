#include "IOCommon.h"

std::vector<uint8_t> FormatToNETASCII(std::vector<uint8_t> data) {
  std::vector<uint8_t> netasciiData;

  for (__u_long i = 0; i < data.size(); ++i) {
    if (data[i] == '\n') {
      // Convert line ending to \r\n
      netasciiData.push_back('\r');
    }
    if (data[i] == '\r') {
      // If a single \r is encountered, replace it with \r\n
      netasciiData.push_back('\r');
    }
    // Add the original character to the output
    netasciiData.push_back(data[i]);
  }

  // Ensure that the last line ends with \r\n
  if (!netasciiData.empty() && netasciiData.back() != '\n') {
    netasciiData.push_back('\r');
  }
  netasciiData.push_back('\n');

  return netasciiData;
}
