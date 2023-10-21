#include "UdpClient.h"

UdpClient::UdpClient(std::string server_hostname, int port_number) {
  // assign the port number
  ChangePort(port_number);

  // get the server address
  struct hostent *server;
  if ((server = gethostbyname(server_hostname.c_str())) == NULL) {
    throw UdpException("Error: Failed to get server address");
  }

  // initialize the server address structure
  memset(&server_address_, 0, sizeof(server_address_));

  server_address_.sin_family = AF_INET;  // IPv4
  // copy the server address to the server_address structure
  memcpy((char *)&server_address_.sin_addr.s_addr, (char *)server->h_addr,
         server->h_length);
  // set the port number
  server_address_.sin_port = htons(port_number_);
  // set the server length

  // create the socket
  if ((client_socket_ = socket(AF_INET, SOCK_DGRAM, 0)) <= 0) {
    throw UdpException("Error: Failed to create socket");
  }
}

UdpClient::~UdpClient() {
  if (client_socket_ > 0) {
    close(client_socket_);
  }
}

void UdpClient::Send(std::vector<uint8_t> data) {
  socklen_t serverlen = sizeof(server_address_);
  // Sending the data
  ssize_t bytes_sent = sendto(client_socket_, data.data(), data.size(), 0,
                              (struct sockaddr *)&server_address_, serverlen);
  if (bytes_sent < 0) {
    throw UdpException("Error: Failed to send data. Error: " +
                       std::to_string(errno));
  }
}

std::vector<uint8_t> UdpClient::ReceiveFromAny(sockaddr_in *sender_address) {
  // Creating a buffer to store the received data
  std::vector<uint8_t> buffer(maxPacketSize_);

  // Capturing the sender's address and port
  socklen_t senderlen = sizeof(*sender_address);

  // Receiving the data and capturing sender's address and port
  ssize_t bytes_received =
      recvfrom(client_socket_, buffer.data(), buffer.size(), 0,
               (struct sockaddr *)sender_address, &senderlen);
  if (bytes_received < 0) {
    if (errno == EWOULDBLOCK) {
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

std::vector<uint8_t> UdpClient::ReceiveFromSpecific() {
  // Creating a buffer to store the received data
  std::vector<uint8_t> buffer(maxPacketSize_);

  // Create a unique_ptr to a sockaddr_in object to store the sender's address
  std::unique_ptr<sockaddr_in> sender_address(new sockaddr_in);

  // Receive data from any sender and capture the sender's address
  buffer = ReceiveFromAny(sender_address.get());

  while (sender_address->sin_port != server_address_.sin_port) {
    // Keep receiving data until the sender's port matches the server's port
    buffer = ReceiveFromAny(sender_address.get());
  }

  return buffer;
}

void UdpClient::ChangePort(int port_number) {
  port_number_ = port_number;
  server_address_.sin_port = htons(port_number);
}

int UdpClient::GetPort() { return port_number_; }

void UdpClient::ChangeMaxPacketSize(uint16_t maxPacketSize) {
  maxPacketSize_ = maxPacketSize;
}

void UdpClient::ChangeTimeout(struct timeval timeout) {
  if (setsockopt(client_socket_, SOL_SOCKET, SO_RCVTIMEO, &timeout,
                 sizeof(timeout)) < 0) {
    throw UdpException("Error: Failed to set socket timeout");
  }
}