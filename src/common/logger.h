/**
* Author: Stefan Peknik
* Mail: xpekni01@vutbr.cz
*/

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

  // RRQ {SRC_IP}:{SRC_PORT} "{FILEPATH}" {MODE} {$OPTS}
  // {$OPTS} = {OPT1_NAME}={OPT1_VALUE} ... {OPTn_NAME}={OPTn_VALUE}
  static void LogRRQ(struct sockaddr_in src_address, ReadWritePacket rrq) {

    std::cerr << "RRQ " << inet_ntoa(src_address.sin_addr) << ":"
              << ntohs(src_address.sin_port) << " \"" << rrq.filepath << "\" "
              << rrq.ModeToString();
    for (auto option : rrq.options) {
      std::cerr << " " << option.nameStr << "=" << option.value;
    }
    std::cerr << std::endl;
  }

  // WRQ {SRC_IP}:{SRC_PORT} "{FILEPATH}" {MODE} {$OPTS}
  // {$OPTS} = {OPT1_NAME}={OPT1_VALUE} ... {OPTn_NAME}={OPTn_VALUE}
  static void logWRQ(struct sockaddr_in src_address, ReadWritePacket wrq) {
    std::cerr << "WRQ " << inet_ntoa(src_address.sin_addr) << ":"
              << ntohs(src_address.sin_port) << " \"" << wrq.filepath << "\" "
              << wrq.ModeToString();
    for (auto option : wrq.options) {
      std::cerr << " " << option.nameStr << "=" << option.value;
    }
    std::cerr << std::endl;
  }

  // ACK {SRC_IP}:{SRC_PORT} {BLOCK_ID}
  static void logACK(struct sockaddr_in src_address, AckPacket ack) {
    std::cerr << "ACK " << inet_ntoa(src_address.sin_addr) << ":"
              << ntohs(src_address.sin_port) << " " << ack.block_number
              << std::endl;
  }

  // OACK {SRC_IP}:{SRC_PORT} {$OPTS}
  // {$OPTS} = {OPT1_NAME}={OPT1_VALUE} ... {OPTn_NAME}={OPTn_VALUE}
  static void logOACK(struct sockaddr_in src_address, OackPacket oack) {
    std::cerr << "OACK " << inet_ntoa(src_address.sin_addr) << ":"
              << ntohs(src_address.sin_port);
    for (auto option : oack.options) {
      std::cerr << " " << option.nameStr << "=" << option.value;
    }
    std::cerr << std::endl;
  }

  // DATA {SRC_IP}:{SRC_PORT}:{DST_PORT} {BLOCK_ID}
  static void logDATA(struct sockaddr_in src_address, int dst_port,
                      DataPacket data) {
    std::cerr << "DATA " << inet_ntoa(src_address.sin_addr) << ":"
              << ntohs(src_address.sin_port) << ":" << dst_port << " "
              << data.block_number << std::endl;
  }

  // ERROR {SRC_IP}:{SRC_PORT}:{DST_PORT} {CODE} "{MESSAGE}"
  static void logERROR(struct sockaddr_in src_address, int dst_port,
                       ErrorPacket error) {
    std::cerr << "ERROR " << inet_ntoa(src_address.sin_addr) << ":"
              << ntohs(src_address.sin_port) << ":" << dst_port << " "
              << int(error.error_code) << " \"" << error.error_message << "\""
              << std::endl;
  }
};

#endif // Logger_h
