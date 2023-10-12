#ifndef OackPacket_h
#define OackPacket_h

#include "Option.h"
#include "TftpPacket.h"

class OackPacket : public TftpPacket {
 public:
  OackPacket();
  OackPacket(std::vector<uint8_t> raw);
  std::vector<Option> options = {};

  std::vector<Option> ParseOptions(std::vector<uint8_t> options);
  std::string ParseOptionName(std::vector<uint8_t> option_name);
  std::string ParseOptionValue(std::vector<uint8_t> option_value);

  std::vector<uint8_t> MakeRaw();
};

#endif  // OackPacket_h
