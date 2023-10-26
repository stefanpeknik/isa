#ifndef TftpPacket_H
#define TftpPacket_H

#include <cstdint>
#include <map>
#include <sstream>
#include <string>
#include <vector>

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

// Exception classes

class TFTPException : public std::exception {
 public:
  TFTPException(const std::string& message) : message(message) {}

  const char* what() const noexcept override { return message.c_str(); }

 private:
  std::string message;
};

class TFTPUnknownError : public TFTPException {  // 0
 public:
  TFTPUnknownError() : TFTPException("Unknown error.") {}
};

class TFTPFileNotFoundError : public TFTPException {  // 1
 public:
  TFTPFileNotFoundError(const std::string& filename)
      : TFTPException("File not found: " + filename) {}
};

class TFTPAccessViolationError : public TFTPException {  // 2
 public:
  TFTPAccessViolationError()
      : TFTPException(
            "Access violation: You don't have permission to access this "
            "file.") {}
};

class TFTPDiskFullError : public TFTPException {  // 3
 public:
  TFTPDiskFullError()
      : TFTPException(
            "Disk full or allocation exceeded: Unable to write the file.") {}
};

class TFTPIllegalOperationError : public TFTPException {  // 4
 public:
  TFTPIllegalOperationError(const std::string& operation)
      : TFTPException("Illegal TFTP operation: " + operation) {}
};

class TFTPUnknownTransferIDError : public TFTPException {
 public:
  TFTPUnknownTransferIDError()
      : TFTPException(
            "Unknown transfer ID: The server did not recognize your request.") {
  }
};

class TFTPFileAlreadyExistsError : public TFTPException {  // 6
 public:
  TFTPFileAlreadyExistsError(const std::string& filename)
      : TFTPException("File already exists: " + filename) {}
};

class TFTPNoSuchUserError : public TFTPException {  // 7
 public:
  TFTPNoSuchUserError()
      : TFTPException(
            "No such user: The user specified does not exist or is not allowed "
            "access.") {}
};

class TTFOptionNegotiationError : public TFTPException {  // 8
 public:
  TTFOptionNegotiationError() : TFTPException("Option negotiation failed.") {}
};

#endif  // TftpPacket_H
