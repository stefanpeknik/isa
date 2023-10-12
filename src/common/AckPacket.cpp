#include "AckPacket.h"

AckPacket::AckPacket() : TftpPacket(TftpPacket::Opcode::ACK) {}

AckPacket::AckPacket(std::vector<uint8_t> raw)
    : TftpPacket(TftpPacket::GetOpcodeFromRaw(raw)) {
  this->block_number =
      ParseBlockNumber(std::vector<uint8_t>(raw.begin() + 2, raw.begin() + 4));
}

uint16_t AckPacket::ParseBlockNumber(std::vector<uint8_t> block_number) {
  return (block_number[0] << 8) + block_number[1];
}

std::vector<uint8_t> AckPacket::MakeRaw() {
  // TODO implement
}