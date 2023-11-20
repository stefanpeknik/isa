/**
* Author: Stefan Peknik
* Mail: xpekni01@vutbr.cz
*/

#ifndef OackPacket_h
#define OackPacket_h

#include "Option.h"
#include "TftpPacket.h"

class OackPacket : public TftpPacket {
public:
  OackPacket(std::vector<Option> options);
  OackPacket(std::vector<uint8_t> raw);
  std::vector<Option> options = {};

  std::vector<Option> ParseOptions(std::vector<uint8_t> options);

  std::vector<uint8_t> MakeRaw();
};

#endif // OackPacket_h
