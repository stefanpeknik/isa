#ifndef Logger_h
#define Logger_h

#include <cstdint>
#include <iostream>
#include <vector>

class Logger {
public:
  static void Log(std::string message) { std::cout << message << std::endl; }

  static void Log(std::string where, std::string message) {
    std::cout << where << ": " << message << std::endl;
  }

  static void LogHexa(std::string what, std::vector<uint8_t> data) {
    std::cout << what << ": ";
    for (auto byte : data) {
      std::cout << std::hex << (int)byte << " ";
    }
    std::cout << std::endl;
  }
};

#endif // Logger_h
