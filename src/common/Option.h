#ifndef OPTION_H
#define OPTION_H

#include <string>

class Option {
 public:
  Option(std::string name, std::string value) : name_(name), value_(value){};
  std::string GetName();
  std::string GetValue();

 private:
  std::string name_;
  std::string value_;
};

#endif  // OPTION_H
