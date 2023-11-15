#ifndef UdpCommon_h
#define UdpCommon_h

#include <exception>
#include <string>

class UdpException : public std::exception {
public:
  UdpException(const std::string &message) : message(message) {}

  const char *what() const noexcept override { return message.c_str(); }

private:
  std::string message;
};

class UdpTimeoutException : public UdpException {
public:
  UdpTimeoutException() : UdpException("Error: Timeout") {}
};

class UdpSigintException : public UdpException {
public:
  UdpSigintException() : UdpException("Error: SIGINT") {}
};

#endif // UdpCommon_h
