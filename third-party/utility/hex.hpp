#pragma once

#ifndef HEX_HPP_
#define HEX_HPP_

#include <string>

#include "types.hpp"

namespace sen {

auto reverse(std::string& str) -> std::string& {
  uint length = str.size();
  uint pivot = length >> 1;
  for(std::string::size_type x = 0, y = length - 1; x < pivot && y >= 0; x++, y--)
    std::swap(str[x], str[y]);

  return str;
}

auto hex(uintmax_t value, long precision = 0, char pad_char = '0') -> std::string {
  std::string buffer;
  static const char *hex_str = "0123456789abcdef";

  do {
    uint n = value & 15;
    buffer.push_back(hex_str[n]);
    value >>= 4;
  } while(value);

  if (buffer.size() < precision) {
    buffer.resize(precision, pad_char);
  }

  reverse(buffer);
  return buffer;
}

// auto pad(const std::string& value, long precision, char pad_char) -> std::string {
//   std::string buffer{value};
//   if(precision) buffer.size(precision, pad_char);
//   return buffer;
// }
}

#endif //HEX_HPP_
