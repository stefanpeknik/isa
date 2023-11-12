#ifndef Logger_h
#define Logger_h

#include <arpa/inet.h>
#include <netinet/in.h>

#include <cstdint>
#include <iostream>
#include <vector>

#include "./TftpPacketStuff/AckPacket.h"
#include "./TftpPacketStuff/DataPacket.h"
#include "./TftpPacketStuff/ErrorPacket.h"
#include "./TftpPacketStuff/OackPacket.h"
#include "./TftpPacketStuff/ReadWritePacket.h"

class Logger {
 public:
  static void Log(std::string message) {
    // std::cout << message << std::endl;
  }

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

  static void LogRRQ(struct sockaddr_in src_address, ReadWritePacket rrq) {
    // RRQ {SRC_IP}:{SRC_PORT} "{FILEPATH}" {MODE} {$OPTS}
    // {$OPTS} = {OPT1_NAME}={OPT1_VALUE} ... {OPTn_NAME}={OPTn_VALUE}
    std::cout << "RRQ " << inet_ntoa(src_address.sin_addr) << ":"
              << ntohs(src_address.sin_port) << " \"" << rrq.filepath << "\" "
              << rrq.ModeToString();
    for (auto option : rrq.options) {
      std::cout << " " << option.nameStr << "=" << option.value;
    }
    std::cout << std::endl;
  }

  static void logWRQ(struct sockaddr_in src_address, ReadWritePacket wrq) {
    // WRQ {SRC_IP}:{SRC_PORT} "{FILEPATH}" {MODE} {$OPTS}
    // {$OPTS} = {OPT1_NAME}={OPT1_VALUE} ... {OPTn_NAME}={OPTn_VALUE}
    std::cout << "WRQ " << inet_ntoa(src_address.sin_addr) << ":"
              << ntohs(src_address.sin_port) << " \"" << wrq.filepath << "\" "
              << wrq.ModeToString();
    for (auto option : wrq.options) {
      std::cout << " " << option.nameStr << "=" << option.value;
    }
    std::cout << std::endl;
  }

  static void logACK(struct sockaddr_in src_address, AckPacket ack) {
    // ACK {SRC_IP}:{SRC_PORT} {BLOCK_ID}
    std::cout << "ACK " << inet_ntoa(src_address.sin_addr) << ":"
              << ntohs(src_address.sin_port) << " " << ack.block_number
              << std::endl;
  }

  static void logOACK(struct sockaddr_in src_address, OackPacket oack) {
    // OACK {SRC_IP}:{SRC_PORT} {$OPTS}
    // {$OPTS} = {OPT1_NAME}={OPT1_VALUE} ... {OPTn_NAME}={OPTn_VALUE}
    std::cout << "OACK " << inet_ntoa(src_address.sin_addr) << ":"
              << ntohs(src_address.sin_port);
    for (auto option : oack.options) {
      std::cout << " " << option.nameStr << "=" << option.value;
    }
    std::cout << std::endl;
  }

  static void logDATA(struct sockaddr_in src_address, int dst_port,
                      DataPacket data) {
    // DATA {SRC_IP}:{SRC_PORT}:{DST_PORT} {BLOCK_ID}
    std::cout << "DATA " << inet_ntoa(src_address.sin_addr) << ":"
              << ntohs(src_address.sin_port) << ":" << dst_port << " "
              << data.block_number << std::endl;
  }

  static void logERROR(struct sockaddr_in src_address, int dst_port,
                       ErrorPacket error) {
    // ERROR {SRC_IP}:{SRC_PORT}:{DST_PORT} {CODE} "{MESSAGE}"
    std::cout << "ERROR " << inet_ntoa(src_address.sin_addr) << ":"
              << ntohs(src_address.sin_port) << ":" << dst_port << " "
              << int(error.error_code) << " \"" << error.error_message << "\""
              << std::endl;
  }
};

#endif  // Logger_h
