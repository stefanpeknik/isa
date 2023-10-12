#ifndef DataPacket_H
#define DataPacket_H

#include "TftpPacket.h"

class DataPacket : public TftpPacket {
 public:
  DataPacket();
  DataPacket(std::vector<uint8_t> raw);
  uint16_t block_number;
  std::vector<uint8_t> data;
  bool is_last;
  uint16_t data_length = 512;

  uint16_t ParseBlockNumber(std::vector<uint8_t> block_number);
  std::vector<uint8_t> ParseData(std::vector<uint8_t>);

  std::vector<uint8_t> MakeRaw();
};

#endif  // DataPacket_H