#ifndef TftpPacket_H
#define TftpPacket_H

#include <map>
#include <string>
#include <vector>

#include "Exceptions.h"

class TftpPacket {
 public:
  enum class Opcode {
    RRQ = 1,    // Read request
    WRQ = 2,    // Write request
    DATA = 3,   // Data packet
    ACK = 4,    // Acknowledgment
    ERROR = 5,  // Error packet
    OACK = 6    // Options Acknowledgment
  };

  TftpPacket(Opcode opcode);
  TftpPacket(std::vector<uint8_t> opcode);

  Opcode opcode;
  static Opcode ParseOpcode(std::vector<uint8_t> opcode);
  static Opcode GetOpcodeFromRaw(std::vector<uint8_t> raw);
  virtual std::vector<uint8_t> MakeRaw() = 0;
};

#endif  // TftpPacket_H
