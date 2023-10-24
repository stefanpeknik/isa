#ifndef Writer_h
#define Writer_h

#include "FileHandler.h"

class Writer : public FileHandler {
 public:
  Writer(std::string filepath, DataFormat mode);
  ~Writer();

  void OpenFile();
  void WriteFile(std::vector<uint8_t> data);

 private:
  std::string filepath_;
  std::fstream file_;
  DataFormat mode_;
};

#endif  // Writer_h
