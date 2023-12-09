/**
* Author: Stefan Peknik
* Mail: xpekni01@vutbr.cz
*/

#ifndef TftpPacketFactory_h
#define TftpPacketFactory_h

#include "TftpCommon.h"
#include <memory>
#include <vector>

class TftpPacketFactory {
public:
  static std::unique_ptr<TftpPacket> CreatePacket(std::vector<uint8_t> raw);
};

#endif // TftpPacketFactory_h
