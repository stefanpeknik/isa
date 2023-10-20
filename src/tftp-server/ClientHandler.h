#ifndef ClientHandler_h
#define ClientHandler_h

#include "../common/IOHandler/IOHandler.h"
#include "../common/TftpCommon.h"
#include "../common/logger.h"
#include "../common/udp/UdpClient/UdpClient.h"

class ClientHandler {
 public:
  ClientHandler(std::string hostname, int port);

  void FollowOnIntroPacket(std::vector<uint8_t> intro_packet);

 private:
  UdpClient udp_client_;
  IOHandler io_handler_;

  void FollowOnRRQ(std::vector<uint8_t> intro_packet);
  void FollowOnWRQ(std::vector<uint8_t> intro_packet);
};

#endif  // ClientHandler_h
