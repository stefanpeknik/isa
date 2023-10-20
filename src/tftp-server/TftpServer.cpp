#include "TftpServer.h"

TftpServer::TftpServer(TftpServerArgs args)
    : args_(args), udp_server_(args.port) {}

void TftpServer::run() {
  while (true) {
    std::vector<uint8_t> data;
    sockaddr_in sender_address;
    while (!udp_server_.Receive(data, sender_address))
      ;  // Wait for a packet to arrive
    std::string client_hostname = inet_ntoa(sender_address.sin_addr);
    int client_port = ntohs(sender_address.sin_port);
    std::async(std::launch::async, &TftpServer::StartCommsWithClient, this,
               client_hostname, client_port, data);
  }
}

void TftpServer::StartCommsWithClient(std::string client_hostname,
                                      int client_port,
                                      std::vector<uint8_t> intro_packet) {
  auto client_handler = ClientHandler(client_hostname, client_port);
  client_handler.FollowOnIntroPacket(intro_packet);
}
