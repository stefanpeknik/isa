#ifndef TftpClient_H
#define TftpClient_H

#include <iostream>
#include <string>

#include "../common/Exceptions.h"
#include "../common/TftpPacket.h"

class TftpClient {
 public:
  struct TftpClientArgs {
    enum class TftpMode { READ, WRITE };

    std::string hostname = "";
    int port = 0;
    std::string filepath = "";
    std::string dest_filepath = "";
    TftpMode mode;
  };

  TftpClient(TftpClientArgs args) : args_(args){};

  void run();

 private:
  TftpClientArgs args_;
};

#endif  // TftpClient_H
