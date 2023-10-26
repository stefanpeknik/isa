#ifndef TftpClient_H
#define TftpClient_H

#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "../common/IO/IO.h"
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

  const int MAX_RETRIES = 5;

 private:
  TftpClientArgs args_;
  UdpClient udp_client_;
  struct sockaddr_in server_address_;

  void Write();
  void Read();

  void SendWrq();
  void SendData();

  void SetupUdpClient();
  std::vector<uint8_t> RecievePacketFromServer();
  void ValidateOptionsInOack(std::vector<Option> oack_options);
};

class RecievedErrorPacketException : public std::exception {
 public:
  RecievedErrorPacketException(std::string error_message)
      : error_message_(error_message) {}

  virtual const char *what() const throw() { return error_message_.c_str(); }

 private:
  std::string error_message_;
};

#endif  // TftpClient_H
