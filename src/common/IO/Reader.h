#ifndef Reader_h
#define Reader_h

#include "FileHandler.h"

class Reader : public FileHandler {
 public:
  Reader(std::string filepath, DataFormat mode);
  ~Reader();

  void OpenFile();
  std::vector<uint8_t> ReadFile(int num_bytes);

 private:
  std::string filepath_;
  std::fstream file_;
  DataFormat mode_;
};

#endif  // Reader_h