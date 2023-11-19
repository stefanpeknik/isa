#include "UdpClient.h"

UdpClient::UdpClient() {
  // create a socket
  if ((client_socket_ = socket(AF_INET, SOCK_DGRAM, 0)) <= 0) {
    throw UdpException("Error: Failed to create socket");
  }

  // set socket timeout
  ChangeTimeout(default_timeout_);
}

UdpClient::~UdpClient() {
  if (client_socket_ > 0) {
    close(client_socket_);
  }
}

void UdpClient::Send(std::vector<uint8_t> data, sockaddr_in reciever_address) {
  // Sending the data
  sendto(client_socket_, data.data(), data.size(), 0,
         (struct sockaddr *)&reciever_address, sizeof(reciever_address));
}

std::vector<uint8_t> UdpClient::Receive(sockaddr_in *sender_address) {
  // Creating a buffer to store the received data
  std::vector<uint8_t> buffer(maxPacketSize_);

  // Capturing the sender's address and port
  socklen_t senderlen = sizeof(*sender_address);

  // if SIGINT was received before receiving, throw an exception
  if (SIGINT_RECEIVED.load() == true) {
    throw UdpSigintException();
  }

  // Receiving the data and capturing sender's address and port
  ssize_t bytes_received =
      recvfrom(client_socket_, buffer.data(), buffer.size(), 0,
               (struct sockaddr *)sender_address, &senderlen);
  if (bytes_received < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      // Handle the timeout
      throw UdpTimeoutException();
    } else {
      // Handle other socket errors
      throw UdpException("Error receiving data");
    }
  }

  // Truncating the buffer to the actual size of the received data
  buffer.resize(bytes_received);

  return buffer;
}

void UdpClient::ChangeMaxPacketSize(uint16_t maxPacketSize) {
  maxPacketSize_ = maxPacketSize;
}

void UdpClient::ChangeTimeout(struct timeval timeout) {
  if (setsockopt(client_socket_, SOL_SOCKET, SO_RCVTIMEO, &timeout,
                 sizeof(timeout)) < 0) {
    throw UdpException("Error: Failed to set socket timeout");
  }
}

void UdpClient::IncreaseTimeout(uint16_t multiplier) {
  timeout_.tv_sec *= multiplier;
  ChangeTimeout(timeout_);
}

void UdpClient::TimeoutReset() {
  timeout_ = default_timeout_;
  ChangeTimeout(timeout_);
}

uint16_t UdpClient::GetLocalPort() {
  struct sockaddr_in local_address;
  socklen_t addr_len = sizeof(local_address);
  if (getsockname(client_socket_, (struct sockaddr *)&local_address,
                  &addr_len) != 0) {
    throw UdpException("Error: Failed to get local port");
  }
  return ntohs(local_address.sin_port);
}
