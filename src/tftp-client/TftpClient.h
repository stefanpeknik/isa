#ifndef TftpClient_H
#define TftpClient_H

#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "../common/IOHandler/IOHandler.h"
#include "../common/TftpCommon.h"
#include "../common/logger.h"
#include "../common/udp/UdpClient/UdpClient.h"

class TftpClient {
 public:
  struct TftpClientArgs {
    enum class TftpMode { READ, WRITE };

    std::string hostname = "";
    int port = 0;
    std::string filepath = "";
    std::string dest_filepath = "";
    TftpMode mode;
    ReadWritePacket::Mode file_mode =
        ReadWritePacket::Mode::OCTET;  // set default to octet
    std::vector<Option> options = {};
  };

  TftpClient(TftpClientArgs args);

  void run();

  const int numRetries = 5;

 private:
  IOHandler io_handler_;
  TftpClientArgs args_;
  UdpClient udp_client_;

  void Write();
  void Read();

  void SendWrq();
  void SendData();

  void SetupUdpClient();
  void ValidateOptionsInOack(std::vector<Option> oack_options);
};

#endif  // TftpClient_H
