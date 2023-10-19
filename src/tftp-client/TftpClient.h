#ifndef TftpClient_H
#define TftpClient_H

#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "../common/TftpCommon.h"
#include "../common/logger.h"

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

  const int numRetries = 5;

private:
  TftpClientArgs args_;
  UdpClient udp_client_;

  void Write();
  void Read();

  void SendWrq();
  void SendData();

  void SetupUdpClient();
  void ValidateOptionsInOack(std::vector<Option> oack_options);

  void WriteOld();
  void ReadOld();
};

class TftpClientException : public std::exception {
public:
  TftpClientException(const std::string &message) : message(message) {}

  const char *what() const noexcept override { return message.c_str(); }

private:
  std::string message;
};

class TftpClientBlockNumberException : public TftpClientException {
public:
  TftpClientBlockNumberException(const std::string &message)
      : TftpClientException(message) {}
};

#endif // TftpClient_H
