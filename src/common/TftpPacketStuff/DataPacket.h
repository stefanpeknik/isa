/**
* Author: Stefan Peknik
* Mail: xpekni01@vutbr.cz
*/

#ifndef DataPacket_H
#define DataPacket_H

#include "TftpPacket.h"

class DataPacket : public TftpPacket {
public:
  DataPacket(uint16_t block_number, std::vector<uint8_t> data,
             int blksize = 512);
  DataPacket(std::vector<uint8_t> raw, int blksize = 512);
  uint16_t block_number;
  std::vector<uint8_t> data;

  uint16_t ParseBlockNumber(std::vector<uint8_t> block_number);
  std::vector<uint8_t> ParseData(std::vector<uint8_t>);

  std::vector<uint8_t> MakeRaw();
};

#endif // DataPacket_H