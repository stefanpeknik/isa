#ifndef Option_H
#define Option_H

#include <algorithm>
#include <cstdint>
#include <exception>
#include <stdexcept>
#include <string>
#include <vector>

#include "utils/utils.h"

class Option {
public:
  enum class Name {
    BLKSIZE,
    TIMEOUT,
    TSIZE,
  };

  Option(std::string name, std::string value);

  Name name;
  std::string nameStr;
  std::string value;

  std::vector<uint8_t> MakeRaw();
};

class OptionException : public std::exception {
public:
  OptionException(const std::string &message) : message(message) {}

  const char *what() const noexcept override { return message.c_str(); }

private:
  std::string message;
};

class UnsupportedOptionException : public OptionException {
public:
  UnsupportedOptionException(const std::string &option_name)
      : OptionException("Unsupported option: " + option_name) {}
};

class InvalidOptionValueException : public OptionException {
public:
  InvalidOptionValueException(const std::string &option_name,
                              const std::string &option_value)
      : OptionException("Invalid value for option " + option_name + ": " +
                        option_value) {}
};

#endif // Option_H
