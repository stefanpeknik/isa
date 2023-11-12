#ifndef ClientHandler_h
#define ClientHandler_h

#include "../common/IO/IO.h"
#include "../common/TftpCommon.h"
#include "../common/logger.h"
#include "../common/udp/UdpClient/UdpClient.h"

class ClientHandler {
 public:
  ClientHandler(std::string hostname, int port, std::string root_dirpath);

  void FollowOnIntroPacket(std::vector<uint8_t> intro_packet);

  const int MAX_RETRIES = 5;

 private:
  UdpClient udp_client_;
  struct sockaddr_in client_address_;
  std::string root_dirpath_;

  void FollowOnRRQ(std::vector<uint8_t> intro_packet);
  void FollowOnWRQ(std::vector<uint8_t> intro_packet);

  std::vector<uint8_t> RecievePacketFromClient();

  std::vector<Option> NegotiateOptions(std::vector<Option> options);

  void LogPotentialTftpPacket(struct sockaddr_in sender_address,
                              std::vector<uint8_t> buffer);
};

class RecievedErrorPacketException : public std::exception {
 public:
  RecievedErrorPacketException(std::string error_message)
      : error_message_(error_message) {}

  virtual const char *what() const throw() { return error_message_.c_str(); }

 private:
  std::string error_message_;
};

#endif  // ClientHandler_h
