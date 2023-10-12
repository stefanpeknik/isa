#ifndef ReadWritePacket_H
#define ReadWritePacket_H

#include <map>
#include <string>
#include <vector>

#include "Exceptions.h"
#include "Option.h"
#include "TftpPacket.h"

class ReadWritePacket : public TftpPacket {
 public:
  ReadWritePacket(TftpPacket::Opcode opcode);
  ReadWritePacket(std::vector<uint8_t> raw);
  enum class Mode { NETASCII, OCTET, MAIL };
  std::string filename;
  Mode mode;
  std::vector<Option> options = {};

  Mode ParseMode(std::vector<uint8_t> mode);
  std::vector<Option> ParseOptions(std::vector<uint8_t> options);
  std::vector<uint8_t> ReadWritePacket::MakeRaw();
};

#endif  // ReadWritePacket_H
