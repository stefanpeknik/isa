#ifndef ErrorPacket_h
#define ErrorPacket_h

#include "TftpPacket.h"

class ErrorPacket : public TftpPacket {
 public:
  ErrorPacket();
  ErrorPacket(std::vector<uint8_t> raw);
  uint16_t error_code;
  std::string error_message;

  uint16_t ParseErrorCode(std::vector<uint8_t> error_code);
  std::string ParseErrorMessage(std::vector<uint8_t> error_message);

  std::vector<uint8_t> MakeRaw();
};

#endif  // ErrorPacket_h
