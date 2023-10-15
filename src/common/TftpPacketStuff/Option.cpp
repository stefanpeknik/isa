#include "Option.h"

/*
  if there is a need to add new options, modify switch in Option constructor,
  IsSupported(), GetNameStr() and in MakeRaw()
*/

// block size limits
const int MIN_BLOCK_SIZE = 8;
const int MAX_BLOCK_SIZE = 65464;

// timeout limits
const int MIN_TIMEOUT = 1;
const int MAX_TIMEOUT = 255;

Option::Option(std::string name, std::string value) {
  // to lower case
  std::transform(name.begin(), name.end(), name.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  // switch (name)
  if (name == "blksize") {
    this->name = Name::BLKSIZE;
  } else if (name == "timeout") {
    this->name = Name::TIMEOUT;
  } else if (name == "tsize") {
    this->name = Name::TSIZE;
    // } else if (name == "new option") { this->name = Name::NEW_OPTION;
  } else {
    throw UnsupportedOptionException(name);
  }

  // store string name for later use
  this->nameStr = name;

  // Validate option value
  switch (this->name) {
  case Name::BLKSIZE:
    try {
      int blockSize = std::stoi(value);
      if (blockSize >= MIN_BLOCK_SIZE && blockSize <= MAX_BLOCK_SIZE) {
        // Valid block size
      } else {
        // Invalid block size
        throw InvalidOptionValueException("BLKSIZE", value);
      }
    } catch (const std::invalid_argument &e) {
      // Invalid format, not an integer
      throw InvalidOptionValueException("BLKSIZE", value);
    }
    break;

  case Name::TIMEOUT:
    try {
      int timeout = std::stoi(value);
      if (timeout >= MIN_TIMEOUT && timeout <= MAX_TIMEOUT) {
        // Valid timeout value
      } else {
        // Invalid timeout value
        throw InvalidOptionValueException("TIMEOUT", value);
      }
    } catch (const std::invalid_argument &e) {
      // Invalid format, not an integer
      throw InvalidOptionValueException("TIMEOUT", value);
    }
    break;

  case Name::TSIZE:
    // no validation needed
    break;

    // case Name::NEW_OPTION: break;
  }

  // Set option value
  this->value = value;
}

std::vector<uint8_t> Option::MakeRaw() {
  std::vector<uint8_t> raw;
  std::string nameStr;

  // switch (this->name)
  switch (this->name) {
  case Name::BLKSIZE:
    raw = StringToVectorNullTerminated("blksize");
    break;

  case Name::TIMEOUT:
    raw = StringToVectorNullTerminated("timeout");
    break;

  case Name::TSIZE:
    raw = StringToVectorNullTerminated("tsize");
    break;

    // case Name::NEW_OPTION:
    //   raw = StringToVectorNullTerminated("new option");
    //   break;
  }

  // Add option value
  auto valueVec = StringToVectorNullTerminated(this->value);
  raw.insert(raw.end(), valueVec.begin(), valueVec.end());

  return raw;
}
