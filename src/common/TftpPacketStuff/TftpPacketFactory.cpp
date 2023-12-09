/**
* Author: Stefan Peknik
* Mail: xpekni01@vutbr.cz
*/

#include "TftpPacketFactory.h"

std::unique_ptr<TftpPacket>
TftpPacketFactory::CreatePacket(std::vector<uint8_t> raw) {
  switch (TftpPacket::GetOpcodeFromRaw(raw)) {
  case TftpPacket::Opcode::RRQ:
    return std::make_unique<ReadWritePacket>(raw);
  case TftpPacket::Opcode::WRQ:
    return std::make_unique<ReadWritePacket>(raw);
  case TftpPacket::Opcode::DATA:
    return std::make_unique<DataPacket>(raw);
  case TftpPacket::Opcode::ACK:
    return std::make_unique<AckPacket>(raw);
  case TftpPacket::Opcode::ERROR:
    return std::make_unique<ErrorPacket>(raw);
  case TftpPacket::Opcode::OACK:
    return std::make_unique<OackPacket>(raw);
  default:
    throw TFTPIllegalOperationError("Illegal opcode");
  }
}
