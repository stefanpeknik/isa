#include "DataPacket.h"

DataPacket::DataPacket(std::vector<uint8_t> raw)
    : TftpPacket(TftpPacket::GetOpcodeFromRaw(raw)) {
  this->block_number =
      ParseBlockNumber(std::vector<uint8_t>(raw.begin() + 2, raw.begin() + 4));
  this->data = ParseData(std::vector<uint8_t>(raw.begin() + 4, raw.end()));

  if (this->data.size() < data_length) {
    this->is_last = true;
  }
}

DataPacket::DataPacket() : TftpPacket(TftpPacket::Opcode::DATA) {}

uint16_t DataPacket::ParseBlockNumber(std::vector<uint8_t> block_number) {
  return (block_number[0] << 8) + block_number[1];
}

std::vector<uint8_t> DataPacket::ParseData(std::vector<uint8_t> data) {
  return data;
}

std::vector<uint8_t> DataPacket::MakeRaw() {
  // TODO implement
}
