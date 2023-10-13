#ifndef ReadWritePacket_H
#define ReadWritePacket_H

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "../OptionStuff/Option.h"
#include "TftpPacket.h"

class ReadWritePacket : public TftpPacket {
 public:
  ReadWritePacket(TftpPacket::Opcode opcode);
  ReadWritePacket(std::vector<uint8_t> raw);
  enum class Mode { NETASCII, OCTET };  // mail is not supported
  std::string filename;
  Mode mode;
  std::vector<Option> options = {};

  Mode ParseMode(std::vector<uint8_t> mode);

  std::vector<Option> ParseOptions(std::vector<uint8_t> options);
  std::string ParseOptionName(std::vector<uint8_t> option_name);
  std::string ParseOptionValue(std::vector<uint8_t> option_value);

  std::vector<uint8_t> MakeRaw();
};

#endif  // ReadWritePacket_H
