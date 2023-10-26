#include "TftpServer.h"

TftpServer::TftpServer(TftpServerArgs args)
    : args_(args), udp_server_(UdpServer(args.port)) {}

void TftpServer::run() {
  while (true) {
    std::vector<uint8_t> data;
    struct sockaddr_in sender_address;
    while (!udp_server_.Receive(data, sender_address))
      ;  // Wait for a packet to arrive
    std::string client_hostname = inet_ntoa(sender_address.sin_addr);
    int client_port = ntohs(sender_address.sin_port);

    auto client_handler =
        std::async(std::launch::async, &TftpServer::StartCommsWithClient, this,
                   client_hostname, client_port, args_.root_dirpath, data);
  }
}

void TftpServer::StartCommsWithClient(std::string client_hostname,
                                      int client_port, std::string root_dirpath,
                                      std::vector<uint8_t> intro_packet) {
  try {
    auto client_handler =
        ClientHandler(client_hostname, client_port, root_dirpath);
    client_handler.FollowOnIntroPacket(intro_packet);
  } catch (const std::exception& e) {
    Logger::Log(e.what());
  }
}
