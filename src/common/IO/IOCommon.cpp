#include "IOCommon.h"

std::vector<uint8_t> FormatToNETASCII(std::vector<uint8_t> data) {
  std::vector<uint8_t> formattedData;

  for (int i = 0; i < data.size(); i++) {
    if (data[i] == '\r') {
      // Check if the next character is '\n'
      if (i + 1 < data.size() && data[i + 1] == '\n') {
        formattedData.push_back('\r');
        formattedData.push_back('\n');
        i++;  // Skip the '\n'
      } else {
        formattedData.push_back('\r');
        formattedData.push_back('\0');
      }
    } else if (data[i] == '\n') {
      formattedData.push_back('\r');
      formattedData.push_back('\n');
    } else {
      formattedData.push_back(data[i]);
    }
  }

  return formattedData;
}
