#ifndef ErrorPacket_h
#define ErrorPacket_h

#include "TftpPacket.h"

class ErrorPacket : public TftpPacket {
 public:
  enum class ErrorCode {
    NOT_DEFINED = 0,
    FILE_NOT_FOUND = 1,
    ACCESS_VIOLATION = 2,
    DISK_FULL = 3,
    ILLEGAL_OPERATION = 4,
    UNKNOWN_TID = 5,
    FILE_ALREADY_EXISTS = 6,
    NO_SUCH_USER = 7,
    FAILED_NEGOTIATION = 8
  };

  ErrorPacket(ErrorCode error_code, std::string error_message);
  ErrorPacket(std::vector<uint8_t> raw);
  ErrorCode error_code;
  std::string error_message;

  ErrorCode ParseErrorCode(std::vector<uint8_t> error_code);
  std::string ParseErrorMessage(std::vector<uint8_t> error_message);

  std::vector<uint8_t> MakeRaw();
};

#endif  // ErrorPacket_h
