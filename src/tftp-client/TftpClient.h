#ifndef TftpClient_H
#define TftpClient_H

#include <iostream>
#include <string>

#include "../common/OptionStuff/Option.h"
#include "../common/TftpPacketStuff/TftpPacket.h"

class TftpClient {
 public:
  struct TftpClientArgs {
    enum class TftpMode { READ, WRITE };

    std::string hostname = "";
    int port = 0;
    std::string filepath = "";
    std::string dest_filepath = "";
    TftpMode mode;
    std::vector<Option> options = {};
  };

  TftpClient(TftpClientArgs args) : args_(args){};

  

  void run();

 private:
  TftpClientArgs args_;

  void Write();
  void Read();
};

#endif  // TftpClient_H
