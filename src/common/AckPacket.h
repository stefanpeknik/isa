#ifndef AckPacket_h
#define AckPacket_h

#include "TftpPacket.h"

class AckPacket : public TftpPacket {
 public:
  AckPacket();
  AckPacket(std::vector<uint8_t> raw);
  uint16_t block_number;

  uint16_t ParseBlockNumber(std::vector<uint8_t> block_number);
  std::vector<uint8_t> MakeRaw();
};

#endif  // AckPacket_h
