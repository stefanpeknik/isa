#ifndef UdpClient_h
#define UdpClient_h

#include <arpa/inet.h>
#include <fcntl.h>
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

#include "../../sigint.h"
#include "../UdpCommon.h"

class UdpClient {
public:
  UdpClient();

  ~UdpClient();

  void Send(std::vector<uint8_t> data, struct sockaddr_in reciever_address);
  std::vector<uint8_t> Receive(sockaddr_in *sender_address);

  void ChangeMaxPacketSize(uint16_t maxPacketSize);
  void ChangeTimeout(struct timeval timeout);
  void IncreaseTimeout(uint16_t multiplier = 2);
  void TimeoutReset();

  uint16_t GetLocalPort();

private:
  int client_socket_;
  struct timeval default_timeout_ = {1, 0};
  struct timeval timeout_ = default_timeout_;
  uint16_t maxPacketSize_ = 65500; // 65507 is the maximum size of a UDP datagram
                                  // (RFC 791), but we need to account for the
                                  // IP header (20 bytes) and UDP header (8
                                  // bytes) and leave some space just in case
};

#endif // UdpClient_h
