#include "TftpPacket.h"

TftpPacket::TftpPacket(std::vector<uint8_t> opcode) {
  this->opcode = ParseOpcode(opcode);
}

TftpPacket::TftpPacket(TftpPacket::Opcode opcode) { this->opcode = opcode; }

TftpPacket::Opcode TftpPacket::ParseOpcode(std::vector<uint8_t> opcode) {
  if (opcode[0] == 0x00 && opcode[1] == 0x01) {
    return Opcode::RRQ;
  } else if (opcode[0] == 0x00 && opcode[1] == 0x02) {
    return Opcode::WRQ;
  } else if (opcode[0] == 0x00 && opcode[1] == 0x03) {
    return Opcode::DATA;
  } else if (opcode[0] == 0x00 && opcode[1] == 0x04) {
    return Opcode::ACK;
  } else if (opcode[0] == 0x00 && opcode[1] == 0x05) {
    return Opcode::ERROR;
  } else if (opcode[0] == 0x00 && opcode[1] == 0x06) {
    return Opcode::OACK;
  } else {
    throw TFTPIllegalOperationError("Invalid opcode: " +
                                    std::string(opcode.begin(), opcode.end()));
  }
}

TftpPacket::Opcode TftpPacket::GetOpcodeFromRaw(std::vector<uint8_t> raw) {
  return TftpPacket::ParseOpcode(
      std::vector<uint8_t>(raw.begin(), raw.begin() + 2));
}
