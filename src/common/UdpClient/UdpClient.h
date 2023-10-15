#ifndef UdpClient_h
#define UdpClient_h

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

class UdpClient {
 public:
  UdpClient(std::string server_hostname, int port_number,
            uint16_t maxPacketSize = 2 + 2 + 512,
            struct timeval timeout = {5, 0});
  ~UdpClient();

  void Send(std::vector<uint8_t> data);
  std::vector<uint8_t> ReceiveFromSpecific();
  std::vector<uint8_t> ReceiveFromAny(sockaddr_in *sender_address);

  void ChangePort(int port_number);
  int GetPort();
  void changeMaxPacketSize(uint16_t maxPacketSize);

 private:
  int client_socket_;
  int port_number_;
  socklen_t serverlen_;
  struct sockaddr_in server_address_;
  int16_t maxPacketSize_;
};

class UdpException : public std::exception {
 public:
  UdpException(const std::string &message) : message(message) {}

  const char *what() const noexcept override { return message.c_str(); }

 private:
  std::string message;
};

class UdpTimeoutException : public UdpException {
 public:
  UdpTimeoutException() : UdpException("Error: Timeout") {}
};

#endif  // UdpClient_h
