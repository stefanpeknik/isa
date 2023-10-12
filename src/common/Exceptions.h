#ifndef Exceptions_H
#define Exceptions_H

#include <exception>
#include <string>

class TFTPException : public std::exception {
 public:
  TFTPException(const std::string& message) : message(message) {}

  const char* what() const noexcept override { return message.c_str(); }

 private:
  std::string message;
};

class TFTPFileNotFoundError : public TFTPException {
 public:
  TFTPFileNotFoundError(const std::string& filename)
      : TFTPException("File not found: " + filename) {}
};

class TFTPAccessViolationError : public TFTPException {
 public:
  TFTPAccessViolationError()
      : TFTPException(
            "Access violation: You don't have permission to access this "
            "file.") {}
};

class TFTPDiskFullError : public TFTPException {
 public:
  TFTPDiskFullError()
      : TFTPException(
            "Disk full or allocation exceeded: Unable to write the file.") {}
};

class TFTPIllegalOperationError : public TFTPException {
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

class TFTPFileAlreadyExistsError : public TFTPException {
 public:
  TFTPFileAlreadyExistsError(const std::string& filename)
      : TFTPException("File already exists: " + filename) {}
};

class TFTPNoSuchUserError : public TFTPException {
 public:
  TFTPNoSuchUserError()
      : TFTPException(
            "No such user: The user specified does not exist or is not allowed "
            "access.") {}
};

class TTFOptionNegotiationError : public TFTPException {
 public:
  TTFOptionNegotiationError() : TFTPException("Option negotiation failed.") {}
};

#endif  // Exceptions_H