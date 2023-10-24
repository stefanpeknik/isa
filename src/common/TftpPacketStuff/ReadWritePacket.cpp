#include "ReadWritePacket.h"

ReadWritePacket::ReadWritePacket(TftpPacket::Opcode opcode,
                                 std::string filepath, Mode mode,
                                 std::vector<Option> options)
    : TftpPacket(opcode), filepath(filepath), mode(mode), options(options) {
  if (opcode != TftpPacket::Opcode::RRQ && opcode != TftpPacket::Opcode::WRQ) {
    throw TFTPIllegalOperationError("Invalid opcode for ReadWritePacket");
  }
}

ReadWritePacket::ReadWritePacket(std::vector<uint8_t> raw)
    : TftpPacket(TftpPacket::GetOpcodeFromRaw(raw)) {
  // Parse filepath
  auto filepathEnd = std::find(raw.begin() + 2, raw.end(), '\0');
  if (filepathEnd == raw.end()) {
    throw TFTPIllegalOperationError("Invalid filepath in packet");
  }
  this->filepath = std::string(raw.begin() + 2, filepathEnd);

  // Parse mode
  auto modeEnd = std::find(filepathEnd + 1, raw.end(), '\0');
  if (modeEnd == raw.end()) {
    throw TFTPIllegalOperationError("Invalid mode in packet");
  }
  this->mode = ParseMode(std::vector<uint8_t>(filepathEnd + 1, modeEnd));

  if (modeEnd != raw.end() - 1) {
    // Parse options
    this->options = ReadWritePacket::ParseOptions(
        std::vector<uint8_t>(modeEnd + 1, raw.end()));
  }
}

ReadWritePacket::Mode ReadWritePacket::ParseMode(std::vector<uint8_t> mode) {
  std::string modeStr(mode.begin(), mode.end());

  // Convert modeStr to lowercase for case-insensitive comparison
  std::transform(modeStr.begin(), modeStr.end(), modeStr.begin(), ::tolower);

  if (modeStr == "netascii") {
    return Mode::NETASCII;
  } else if (modeStr == "octet") {
    return Mode::OCTET;
  } else {
    throw TFTPIllegalOperationError("Invalid mode: " + modeStr);
  }
}

std::vector<Option> ReadWritePacket::ParseOptions(
    std::vector<uint8_t> options) {
  std::vector<Option> parsed_options = {};

  for (size_t i = 0; i < options.size();) {
    // Read option name
    std::string option_name;
    while (i < options.size() && options[i] != '\0') {
      option_name += std::tolower(options[i]);
      i++;
    }
    if (i == options.size()) {
      throw TFTPIllegalOperationError(
          "TFTP Illegal Operation: option name not terminated with null byte");
    }
    i++;  // Skip null byte

    // Read option value
    std::string option_value;
    while (i < options.size() && options[i] != '\0') {
      option_value += std::tolower(options[i]);
      i++;
    }
    if (i == options.size()) {
      throw TFTPIllegalOperationError(
          "TFTP Illegal Operation: option value not terminated with null byte");
    }
    i++;  // Skip null byte

    // Check if option value is missing
    if (option_value.empty()) {
      throw TFTPIllegalOperationError(
          "TFTP Illegal Operation: missing value for option " + option_name);
    }

    // Create new Option object
    Option new_option(option_name, option_value);

    // Check if option name is already in parsed_options
    for (auto option : parsed_options) {
      if (option.name == new_option.name) {
        throw TFTPIllegalOperationError(
            "TFTP Illegal Operation: duplicate option name " + option_name);
      }
    }

    // Add new option to parsed_options
    parsed_options.push_back(new_option);
  }

  return parsed_options;
}

std::string ReadWritePacket::ParseOptionName(std::vector<uint8_t> option_name) {
  return std::string(option_name.begin(), option_name.end());
}

std::string ReadWritePacket::ParseOptionValue(
    std::vector<uint8_t> option_value) {
  return std::string(option_value.begin(), option_value.end());
}

std::vector<uint8_t> ReadWritePacket::MakeRaw() {
  std::vector<uint8_t> raw;

  // Add opcode
  raw.push_back(static_cast<uint8_t>(this->opcode) >> 8);
  raw.push_back(static_cast<uint8_t>(this->opcode));

  // Add filepath
  auto filepathVec = StringToVectorNullTerminated(this->filepath);
  raw.insert(raw.end(), filepathVec.begin(), filepathVec.end());

  // Add mode
  auto modeVec = StringToVectorNullTerminated(ModeToString(this->mode));
  raw.insert(raw.end(), modeVec.begin(), modeVec.end());

  // Add options
  for (auto &option : this->options) {
    auto optionVec = option.MakeRaw();
    raw.insert(raw.end(), optionVec.begin(), optionVec.end());
  }

  return raw;
}

std::string ReadWritePacket::ModeToString(ReadWritePacket::Mode mode) {
  switch (mode) {
    case ReadWritePacket::Mode::NETASCII:
      return "netascii";
      break;

    case ReadWritePacket::Mode::OCTET:
      return "octet";
      break;
  }

  // dummy return
  return "netascii";
}
