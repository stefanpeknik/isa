#include "ErrorPacket.h"

ErrorPacket::ErrorPacket(ErrorCode error_code, std::string error_message)
    : TftpPacket(TftpPacket::Opcode::ERROR), error_code(error_code),
      error_message(error_message) {}

ErrorPacket::ErrorPacket(std::vector<uint8_t> raw)
    : TftpPacket(TftpPacket::GetOpcodeFromRaw(raw)) {
  this->error_code =
      ParseErrorCode(std::vector<uint8_t>(raw.begin() + 2, raw.begin() + 4));
  this->error_message =
      ParseErrorMessage(std::vector<uint8_t>(raw.begin() + 4, raw.end() - 1));
}

ErrorPacket::ErrorCode
ErrorPacket::ParseErrorCode(std::vector<uint8_t> error_code) {
  auto err_code_int = (error_code[0] << 8) + error_code[1];
  switch (err_code_int) {
  case 0:
    return ErrorCode::NOT_DEFINED;
  case 1:
    return ErrorCode::FILE_NOT_FOUND;
  case 2:
    return ErrorCode::ACCESS_VIOLATION;
  case 3:
    return ErrorCode::DISK_FULL;
  case 4:
    return ErrorCode::ILLEGAL_OPERATION;
  case 5:
    return ErrorCode::UNKNOWN_TID;
  case 6:
    return ErrorCode::FILE_ALREADY_EXISTS;
  case 7:
    return ErrorCode::NO_SUCH_USER;
  case 8:
    return ErrorCode::FAILED_NEGOTIATION;
  default:
    throw TFTPIllegalOperationError("Invalid error code: " +
                                    std::to_string(err_code_int));
  }
}

std::string ErrorPacket::ParseErrorMessage(std::vector<uint8_t> error_message) {
  return std::string(error_message.begin(), error_message.end());
}

std::vector<uint8_t> ErrorPacket::MakeRaw() {
  std::vector<uint8_t> raw;

  // Add opcode
  raw.push_back(static_cast<uint8_t>(this->opcode) >> 8);
  raw.push_back(static_cast<uint8_t>(this->opcode));

  // Add error code
  raw.push_back(static_cast<uint8_t>(this->error_code) >> 8);
  raw.push_back(static_cast<uint8_t>(this->error_code));

  // Add error message
  raw.insert(raw.end(), this->error_message.begin(), this->error_message.end());

  // Add null terminator
  raw.push_back(0);

  return raw;
}
