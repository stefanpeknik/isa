#include "ReadWritePacket.h"

ReadWritePacket::ReadWritePacket(TftpPacket::Opcode opcode)
    : TftpPacket(opcode) {}

ReadWritePacket::ReadWritePacket(std::vector<uint8_t> raw)
    : TftpPacket(TftpPacket::GetOpcodeFromRaw(raw)) {
  // Parse filename
  auto filenameEnd = std::find(raw.begin() + 2, raw.end(), '\0');
  if (filenameEnd == raw.end()) {
    throw TFTPIllegalOperationError("Invalid filename in packet");
  }
  this->filename = std::string(raw.begin() + 2, filenameEnd);

  // Parse mode
  auto modeEnd = std::find(filenameEnd + 1, raw.end(), '\0');
  if (modeEnd == raw.end()) {
    throw TFTPIllegalOperationError("Invalid mode in packet");
  }
  this->mode = ParseMode(std::vector<uint8_t>(filenameEnd + 1, modeEnd));

  if (modeEnd != raw.end() - 1) {
    // Parse options
    this->options = ReadWritePacket::ParseOptions(
        std::vector<uint8_t>(modeEnd + 1, raw.end()));
  }
}

ReadWritePacket::Mode ReadWritePacket::ParseMode(std::vector<uint8_t> mode) {
  std::string modeStr;
  for (uint8_t byte : mode) {
    modeStr += static_cast<char>(byte);
  }

  // Convert modeStr to lowercase for case-insensitive comparison
  std::transform(modeStr.begin(), modeStr.end(), modeStr.begin(), ::tolower);

  if (modeStr == "netascii") {
    return Mode::NETASCII;
  } else if (modeStr == "octet") {
    return Mode::OCTET;
  } else if (modeStr == "mail") {
    return Mode::MAIL;
  } else {
    throw TFTPIllegalOperationError("Invalid mode: " + modeStr);
  }
}

std::vector<Option> ReadWritePacket::ParseOptions(
    std::vector<uint8_t> options) {}

std::vector<uint8_t> ReadWritePacket::MakeRaw() {
  // TODO implement
}
