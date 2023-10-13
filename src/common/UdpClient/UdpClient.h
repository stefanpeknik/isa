#ifndef UdpClient_h
#define UdpClient_h

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <cstring>
#include <string>
#include <vector>

class UdpClient {
 public:
  UdpClient(std::string server_hostname, int port_number,
            int16_t maxPacketSize = 2 + 2 + 512,
            struct timeval timeout = {5, 0});

  void Send(std::vector<int8_t> data);
  std::vector<int8_t> Receive();

  void ChangePort(int port_number);
  void changeMaxPacketSize(int16_t maxPacketSize);

 private:
  int client_socket_;
  int port_number_;
  socklen_t serverlen_;
  struct sockaddr_in server_address_;
  int16_t maxPacketSize_;
};

class UdpException : public std::exception {
 public:
  UdpException(const std::string& message) : message(message) {}

  const char* what() const noexcept override { return message.c_str(); }

 private:
  std::string message;
};

#endif  // UdpClient_h
