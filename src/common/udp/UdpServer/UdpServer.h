#ifndef UdpServer_h
#define UdpServer_h

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

#include "../UdpCommon.h"

class UdpServer {
 public:
  UdpServer(int port_number);
  ~UdpServer();

  bool Receive(std::vector<uint8_t> &data, sockaddr_in &sender_address);
  void ChangeTimeout(struct timeval timeout);
  void ChangeMaxPacketSize(uint16_t maxPacketSize);

 private:
  int server_socket_;
  int port_number_;
  struct sockaddr_in server_address_;
  socklen_t sender_address_len;
  int16_t maxPacketSize_ = 1024; // 
};

#endif  // UdpServer_h
