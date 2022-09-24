#pragma once

#ifndef REG_TYPE_HPP_
#define REG_TYPE_HPP_

#include "embbed-bit-range.hpp"

namespace sen {

using r8 = uint8_t;

union r16 {
  uint16_t w;
  EmbedBitRange<0, 8, uint16_t> lo;
  EmbedBitRange<8, 8, uint16_t> hi;
};

union r24 {
  inline r24() = default;
  inline r24(uint data) : d(data) {}                // NOLINT(google-explicit-constructor)
  inline r24(const r24 &data) : d(data.d) {}
  inline auto operator=(uint data) -> r24& {
    d = data;
    return *this;
  }
  inline auto operator=(const r24 &data) -> r24& {
    d = data.d;
    return *this;
  }

  operator uint32_t() const { return d & 0x00ffffff; } // NOLINT(google-explicit-constructor)

  uint32_t d{};
  EmbedBitRange<0, 8, uint32_t> lo;     // 0-7   bit
  EmbedBitRange<8, 8, uint32_t> hi;     // 8-15  bit
  EmbedBitRange<16, 8, uint32_t> b;      // 16-23 bit
  EmbedBitRange<0, 16, uint32_t> w;      // 0-15  bit
};

}

#endif //REG_TYPE_HPP_
