#ifndef TftpServer_h
#define TftpServer_h

#include <cstdint>
#include <fstream>
#include <future>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "../common/logger.h"
#include "../common/udp/UdpServer/UdpServer.h"
#include "ClientHandler.h"

class TftpServer {
 public:
  struct TftpServerArgs {
    int port = 0;
    std::string root_dirpath = "";
  };

  TftpServer(TftpServerArgs args);

  void run();

  const int numRetries = 5;

 private:
  TftpServerArgs args_;
  UdpServer udp_server_;

  void StartCommsWithClient(std::string client_hostname, int client_port,
                            std::vector<uint8_t> intro_packet);
};

#endif  // TftpServer_h