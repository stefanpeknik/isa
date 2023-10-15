#include "AckPacket.h"

AckPacket::AckPacket(uint16_t block_number)
    : TftpPacket(TftpPacket::Opcode::ACK), block_number(block_number) {}

AckPacket::AckPacket(std::vector<uint8_t> raw)
    : TftpPacket(TftpPacket::GetOpcodeFromRaw(raw)) {
  this->block_number =
      ParseBlockNumber(std::vector<uint8_t>(raw.begin() + 2, raw.begin() + 4));
}

uint16_t AckPacket::ParseBlockNumber(std::vector<uint8_t> block_number) {
  return (block_number[0] << 8) + block_number[1];
}

std::vector<uint8_t> AckPacket::MakeRaw() {
  std::vector<uint8_t> raw;
  raw.push_back(0);
  raw.push_back(static_cast<uint8_t>(this->opcode));
  raw.push_back(static_cast<uint8_t>(this->block_number >> 8));
  raw.push_back(static_cast<uint8_t>(this->block_number));
  return raw;
}
