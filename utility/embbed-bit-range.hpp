#pragma once

#ifndef FC_EMULATOR_THIRD_PARTY_NATURAL_HPP_
#define FC_EMULATOR_THIRD_PARTY_NATURAL_HPP_

#include <type_traits>
#include "types.hpp"

namespace sen {

/*
 * beg the number of data to start from
 * bits contains several bits of data
 * Type type
 */
template<uint beg, uint bits = 1, typename Type = uint8_t>
struct EmbedBitRange {
  enum { precision = sizeof(Type) * 8 };
  enum : Type { mask = ~0llu >> (64 - bits) << beg };
  enum : Type { shift = beg };

  static_assert(beg >= 0 && bits > 0 && beg + bits <= precision, "Number bits isn't true");

  // EmbedBitRange() : target(0) {}
  // EmbedBitRange(const EmbedBitRange &source) : target(0) { cast(Type(source)); return *this; }
  // auto operator=(const EmbedBitRange &rhs) -> EmbedBitRange& { cast(Type(rhs)); return *this; }
  inline EmbedBitRange() = default;
  inline EmbedBitRange(uint16_t data) : target(0) { cast(data); }          // NOLINT(google-explicit-constructor)
  inline EmbedBitRange(const EmbedBitRange& data) : target(0) { cast(Type(data)); }
  inline EmbedBitRange(EmbedBitRange&& data) noexcept : target(0) { cast(Type(data)); }

  inline auto operator=(uint16_t data) -> EmbedBitRange & { cast(data); return *this; }
  inline auto operator=(const EmbedBitRange& data)-> EmbedBitRange& { cast(Type(data)); return *this; }
  operator Type() const { return (target & mask) >> shift; }

  inline auto operator++() -> EmbedBitRange& { cast(*this + 1); return *this; }
  inline auto operator--() -> EmbedBitRange& { cast(*this - 1); return *this; }
  inline auto operator++(int) -> Type { auto value = *this; cast(*this + 1); return value; }
  inline auto operator--(int) -> Type { auto value = *this; cast(*this - 1); return value; }

  template<typename T> inline auto operator  =(const T &source) -> EmbedBitRange& { cast(         source); return *this; }
  template<typename T> inline auto operator *=(const T &source) -> EmbedBitRange& { cast(*this  * source); return *this; }
  template<typename T> inline auto operator /=(const T &source) -> EmbedBitRange& { cast(*this  / source); return *this; }
  template<typename T> inline auto operator %=(const T &source) -> EmbedBitRange& { cast(*this  % source); return *this; }
  template<typename T> inline auto operator +=(const T &source) -> EmbedBitRange& { cast(*this  + source); return *this; }
  template<typename T> inline auto operator -=(const T &source) -> EmbedBitRange& { cast(*this  - source); return *this; }
  template<typename T> inline auto operator &=(const T &source) -> EmbedBitRange& { cast(*this  & source); return *this; }
  template<typename T> inline auto operator ^=(const T &source) -> EmbedBitRange& { cast(*this  ^ source); return *this; }
  template<typename T> inline auto operator |=(const T &source) -> EmbedBitRange& { cast(*this  | source); return *this; }
  template<typename T> inline auto operator<<=(const T &source) -> EmbedBitRange& { cast(*this << source); return *this; }
  template<typename T> inline auto operator>>=(const T &source) -> EmbedBitRange& { cast(*this >> source); return *this; }

 private:
  inline auto cast(Type value) -> void { target = (target & ~mask) | ((value << shift) & mask); }
  Type target;
};
}

#endif //FC_EMULATOR_THIRD_PARTY_NATURAL_HPP_ -> EmbedBitRange&