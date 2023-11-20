/**
* Author: Stefan Peknik
* Mail: xpekni01@vutbr.cz
*/

#include "utils.h"

std::vector<uint8_t> StringToVectorNullTerminated(std::string str) {
  std::vector<uint8_t> vec;
  for (char c : str) {
    vec.push_back(c);
  }
  vec.push_back(0x00);
  return vec;
}
