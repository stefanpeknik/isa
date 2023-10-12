#include "OackPacket.h"

OackPacket::OackPacket() : TftpPacket(TftpPacket::Opcode::OACK) {}

OackPacket::OackPacket(std::vector<uint8_t> raw)
    : TftpPacket(TftpPacket::GetOpcodeFromRaw(raw)) {
  this->options =
      ParseOptions(std::vector<uint8_t>(raw.begin() + 2, raw.end()));
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
    for (auto& option : parsed_options) {
      if (option.GetName() == new_option.GetName()) {
        throw TFTPIllegalOperationError(
            "TFTP Illegal Operation: duplicate option name " +
            new_option.GetName());
      }
    }

    // Add new option to parsed_options
    parsed_options.push_back(new_option);
  }

  return parsed_options;
}

std::string OackPacket::ParseOptionName(std::vector<uint8_t> option_name) {
  return std::string(option_name.begin(), option_name.end());
}

std::string OackPacket::ParseOptionValue(std::vector<uint8_t> option_value) {
  return std::string(option_value.begin(), option_value.end());
}

std::vector<uint8_t> OackPacket::MakeRaw() {
  // TODO implement
}
