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

#include "../UdpCommon.h"

class UdpClient {
 public:
  UdpClient(std::string server_hostname, int port_number);

  ~UdpClient();

  void Send(std::vector<uint8_t> data);
  std::vector<uint8_t> ReceiveFromSpecific();
  std::vector<uint8_t> ReceiveFromAny(sockaddr_in *sender_address);

  void ChangePort(int port_number);
  int GetPort();
  void ChangeMaxPacketSize(uint16_t maxPacketSize);
  void ChangeTimeout(struct timeval timeout);

 private:
  int client_socket_;
  int port_number_;
  struct sockaddr_in server_address_;
  int16_t maxPacketSize_ =
      2 + 2 +
      512;  // the largest packet size in the TFTP protocol is 516 bytes: opcode
            // (2 bytes) + block number (2 bytes) + data (512 bytes)
};

#endif  // UdpClient_h