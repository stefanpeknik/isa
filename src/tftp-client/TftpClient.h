#ifndef TftpClient_H
#define TftpClient_H

#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "../common/TftpCommon.h"

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

  TftpClient(TftpClientArgs args);

  void run();

 private:
  TftpClientArgs args_;
  UdpClient udp_client_;

  void Write();
  void Read();

  void SetupUdpClient();
  void ValidateOptionsInOack(std::vector<Option> oack_options);
};

class TftpClientException : public std::exception {
 public:
  TftpClientException(const std::string &message) : message(message) {}

  const char *what() const noexcept override { return message.c_str(); }

 private:
  std::string message;
};

#endif  // TftpClient_H
