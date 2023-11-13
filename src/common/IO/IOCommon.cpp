#include "IOCommon.h"

std::vector<uint8_t> FormatToNETASCII(std::vector<uint8_t> data) {
  std::vector<uint8_t> formattedData;

  for (size_t i = 0; i < data.size(); i++) {
    if (data[i] == '\r') {
      // Check if the next character is '\n'
      if (i + 1 < data.size() && data[i + 1] == '\n') {
        formattedData.push_back('\r');
        formattedData.push_back('\n');
        i++; // Skip the '\n'
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

std::vector<uint8_t> FormatFromNETASCII(std::vector<uint8_t> data) {
  std::vector<uint8_t> result;
  for (size_t i = 0; i < data.size(); ++i) {
    if (i < data.size() - 1 && data[i] == '\r') {
      if (data[i + 1] == '\n') {
        result.push_back('\n');
        ++i; // Skip next element
      } else if (data[i + 1] == '\0') {
        result.push_back('\r');
        ++i; // Skip next element
      } else {
        result.push_back(data[i]);
      }
    } else {
      result.push_back(data[i]);
    }
  }
  return result;
}
