/**
 * Author: Stefan Peknik
 * Mail: xpekni01@vutbr.cz
 */

#include "DataPacket.h"

DataPacket::DataPacket(uint16_t block_number, std::vector<uint8_t> data,
                       int blksize)
    : TftpPacket(TftpPacket::Opcode::DATA), block_number(block_number),
      data(std::vector<uint8_t>(data.begin(), data.end())) {}

DataPacket::DataPacket(std::vector<uint8_t> raw, int blksize)
    : TftpPacket(TftpPacket::GetOpcodeFromRaw(raw)) {
  if (opcode != TftpPacket::Opcode::DATA) {
    throw TFTPIllegalOperationError("Invalid opcode for DataPacket");
  }
  if (raw.size() < 4) {
    throw TFTPIllegalOperationError("Invalid DATA packet");
  }
  if (raw.size() >
      (long unsigned int)(2 + 2 + blksize)) { // opcode + blknum + blksize
    raw.resize(2 + 2 + blksize);
  }
  this->block_number =
      ParseBlockNumber(std::vector<uint8_t>(raw.begin() + 2, raw.begin() + 4));
  this->data = ParseData(std::vector<uint8_t>(raw.begin() + 4, raw.end()));
}

uint16_t DataPacket::ParseBlockNumber(std::vector<uint8_t> block_number) {
  return (block_number[0] << 8) + block_number[1];
}

std::vector<uint8_t> DataPacket::ParseData(std::vector<uint8_t> data) {
  return data;
}

std::vector<uint8_t> DataPacket::MakeRaw() {
  std::vector<uint8_t> raw;

  // Add opcode
  raw.push_back(static_cast<uint8_t>(this->opcode) >> 8);
  raw.push_back(static_cast<uint8_t>(this->opcode));

  // Add block number
  raw.push_back(this->block_number >> 8);
  raw.push_back(this->block_number);

  // Add data
  raw.insert(raw.end(), this->data.begin(), this->data.end());

  return raw;
}
