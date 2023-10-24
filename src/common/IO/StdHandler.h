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
  std::vector<uint8_t> ReadBytesFromStdin(size_t size);

 private:
  DataFormat mode_;
};

#endif  // StdHandler_h