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
  // TODO implement
}
