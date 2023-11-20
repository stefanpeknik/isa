/**
* Author: Stefan Peknik
* Mail: xpekni01@vutbr.cz
*/

#ifndef IOCommon_h
#define IOCommon_h

#include <cstdint>
#include <vector>

enum class DataFormat {
  NETASCII,
  OCTET,
};

std::vector<uint8_t> FormatToNETASCII(std::vector<uint8_t> data);
std::vector<uint8_t> FormatFromNETASCII(std::vector<uint8_t> data);

#endif  // IOCommon_h