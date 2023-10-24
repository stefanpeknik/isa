#ifndef ReadWritePacket_H
#define ReadWritePacket_H

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "../utils/utils.h"
#include "Option.h"
#include "TftpPacket.h"


class ReadWritePacket : public TftpPacket {
 public:
  enum class Mode { NETASCII, OCTET };  // mail is not supported

  ReadWritePacket(TftpPacket::Opcode opcode, std::string filepath, Mode mode,
                  std::vector<Option> options = {});
  ReadWritePacket(std::vector<uint8_t> raw);
  std::string filepath;
  Mode mode;
  std::vector<Option> options = {};

  Mode ParseMode(std::vector<uint8_t> mode);

  std::vector<Option> ParseOptions(std::vector<uint8_t> options);
  std::string ParseOptionName(std::vector<uint8_t> option_name);
  std::string ParseOptionValue(std::vector<uint8_t> option_value);

  std::vector<uint8_t> MakeRaw();

 private:
  std::string ModeToString(ReadWritePacket::Mode mode);
};

#endif  // ReadWritePacket_H
