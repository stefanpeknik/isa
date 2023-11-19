#include "OackPacket.h"

OackPacket::OackPacket(std::vector<Option> options)
    : TftpPacket(TftpPacket::Opcode::OACK), options(options) {}

OackPacket::OackPacket(std::vector<uint8_t> raw)
    : TftpPacket(TftpPacket::GetOpcodeFromRaw(raw)) {
  if (opcode != TftpPacket::Opcode::OACK) {
    throw TFTPIllegalOperationError("Invalid opcode for OackPacket");
  }
  if (raw.size() < 2) {
    throw TFTPIllegalOperationError("Invalid OACK packet");
  }
  try {
    this->options =
        ParseOptions(std::vector<uint8_t>(raw.begin() + 2, raw.end()));
  } catch (InvalidOptionValueException &e) {
      throw TTFOptionNegotiationError();
  }
}

std::vector<Option> OackPacket::ParseOptions(std::vector<uint8_t> options) {
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
    i++; // Skip null byte

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
    i++; // Skip null byte

    // Check if option value is missing
    if (option_value.empty()) {
      throw TFTPIllegalOperationError(
          "TFTP Illegal Operation: missing value for option " + option_name);
    }

    // Create new Option object
    Option new_option(option_name, option_value);

    // Check if option name is already in parsed_options
    for (auto &option : parsed_options) {
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

std::vector<uint8_t> OackPacket::MakeRaw() {
  std::vector<uint8_t> raw;

  // Add opcode
  raw.push_back(static_cast<uint8_t>(this->opcode) >> 8);
  raw.push_back(static_cast<uint8_t>(this->opcode));

  // Add options
  for (auto &option : this->options) {
    auto optionVec = option.MakeRaw();
    raw.insert(raw.end(), optionVec.begin(), optionVec.end());
  }

  return raw;
}
