/**
* Author: Stefan Peknik
* Mail: xpekni01@vutbr.cz
*/

#ifndef StdHandler_h
#define StdHandler_h

#include <iostream>
#include <string>
#include <vector>

#include "IOCommon.h"

class StdHandler {
 public:
  StdHandler(DataFormat mode);

  void WriteBytesToStdout(std::vector<uint8_t> data);
  std::vector<uint8_t> ReadBytesFromStdin(unsigned int num_bytes);

 private:
  DataFormat mode_;
  std::vector<uint8_t> overflow_buffer_;
};

class FailedToReadFromStdinException : public std::exception {
 public:
  FailedToReadFromStdinException() {}

  virtual const char* what() const throw() {
    return "Failed to read from stdin";
  }
};

#endif  // StdHandler_h