#include "UdpClient.h"

UdpClient::UdpClient(std::string server_hostname, int port_number,
                     int16_t maxPacketSize, struct timeval timeout) {
  // assign the port number
  port_number_ = port_number;

  // get the server address
  struct hostent *server;
  if ((server = gethostbyname(server_hostname.c_str())) == NULL) {
    throw UdpException("Error: Failed to get server address");
  }

  // create the socket
  if ((client_socket_ = socket(AF_INET, SOCK_DGRAM, 0)) <= 0) {
    throw UdpException("Error: Failed to create socket");
  }

  // set the timeout
  if (setsockopt(client_socket_, SOL_SOCKET, SO_RCVTIMEO, &timeout,
                 sizeof(timeout)) < 0) {
    throw UdpException("Error: Failed to set socket timeout");
  }

  // initialize the server address structure
  bzero((char *)&server_address_, sizeof(server_address_));
  server_address_.sin_family = AF_INET;  // IPv4
  // copy the server address to the server_address structure
  bcopy((char *)server->h_addr, (char *)&server_address_.sin_addr.s_addr,
        server->h_length);
  // set the port number
  server_address_.sin_port = htons(port_number_);
  maxPacketSize_ = maxPacketSize;
}

void UdpClient::Send(std::vector<int8_t> data) {
  // Sending the data
  ssize_t bytes_sent = sendto(client_socket_, data.data(), data.size(), 0,
                              (struct sockaddr *)&server_address_, serverlen_);
  if (bytes_sent < 0) {
    throw UdpException("Error: Failed to send data");
  }
}

std::vector<int8_t> UdpClient::Receive() {
  // Creating a buffer to store the received data
  std::vector<int8_t> buffer(maxPacketSize_);

  // Receiving the data
  ssize_t bytes_received =
      recvfrom(client_socket_, buffer.data(), buffer.size(), 0,
               (struct sockaddr *)&server_address_, &serverlen_);
  if (bytes_received < 0) {
    throw UdpException("Error: Failed to receive data");
  }

  // Truncating the buffer to the actual size of the received data
  buffer.resize(bytes_received);

  return buffer;
}

void UdpClient::ChangePort(int port_number) {
  port_number_ = port_number;
  server_address_.sin_port = htons(port_number);
}

void UdpClient::changeMaxPacketSize(int16_t maxPacketSize) {
  maxPacketSize_ = maxPacketSize;
}
