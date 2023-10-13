#include "ErrorPacket.h"

ErrorPacket::ErrorPacket() : TftpPacket(TftpPacket::Opcode::ERROR) {}

ErrorPacket::ErrorPacket(std::vector<uint8_t> raw)
    : TftpPacket(TftpPacket::GetOpcodeFromRaw(raw)) {
  this->error_code =
      ParseErrorCode(std::vector<uint8_t>(raw.begin() + 2, raw.begin() + 4));
  this->error_message =
      ParseErrorMessage(std::vector<uint8_t>(raw.begin() + 4, raw.end() - 1));
}

uint16_t ErrorPacket::ParseErrorCode(std::vector<uint8_t> error_code) {
  return (error_code[0] << 8) + error_code[1];
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
  raw.push_back(this->error_code >> 8);
  raw.push_back(this->error_code);

  // Add error message
  raw.insert(raw.end(), this->error_message.begin(), this->error_message.end());

  // Add null terminator
  raw.push_back(0);

  return raw;
}
