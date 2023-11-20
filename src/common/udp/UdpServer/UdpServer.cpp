/**
* Author: Stefan Peknik
* Mail: xpekni01@vutbr.cz
*/

#include "UdpServer.h"

UdpServer::UdpServer(int port_number) {
  port_number_ = port_number;

  server_socket_ = socket(AF_INET, SOCK_DGRAM, 0);
  if (server_socket_ == -1) {
    throw UdpException("Error: Failed to create socket");
  }

  server_address_.sin_family = AF_INET;
  server_address_.sin_port = htons(port_number_);
  server_address_.sin_addr.s_addr = INADDR_ANY;

  if (bind(server_socket_, (struct sockaddr *)&server_address_,
           sizeof(server_address_)) == -1) {
    throw UdpException("Error: Failed to bind socket");
  }
}

UdpServer::~UdpServer() { close(server_socket_); }

bool UdpServer::Receive(std::vector<uint8_t> &data,
                        struct sockaddr_in &sender_address) {
  socklen_t sender_address_len = sizeof(sender_address);
  data = std::vector<uint8_t>(maxPacketSize_);
  if (SIGINT_RECEIVED.load() == true)
    return false; // If SIGINT was received before receiving, return false
  ssize_t bytes_received =
      recvfrom(server_socket_, data.data(), data.size(), 0,
               (struct sockaddr *)&sender_address, &sender_address_len);

  if (bytes_received == -1)
    return false;

  data.resize(bytes_received);
  return true;
}

void UdpServer::ChangeTimeout(timeval timeout) {
  if (setsockopt(server_socket_, SOL_SOCKET, SO_RCVTIMEO, &timeout,
                 sizeof(timeout)) == -1) {
    throw UdpException("Error: Failed to set timeout");
  }
}

void UdpServer::ChangeMaxPacketSize(uint16_t maxPacketSize) {
  maxPacketSize_ = maxPacketSize;
}
