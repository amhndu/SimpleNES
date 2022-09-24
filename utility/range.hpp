#pragma once

#ifndef RANGE_HPP_
#define RANGE_HPP_

#include "types.hpp"

namespace sen {

struct Range {
  struct Iterator {
    explicit Iterator(int64_t position, int64_t step = 0) : position(position), step(step) {}
    auto operator*() const -> int64_t { return position; }
    auto operator!=(const Iterator& source) const -> bool { return step > 0 ? position < source.position : position > source.position; }
    auto operator++() -> Iterator& { position += step; return *this; }

   private:
    int64_t position;
    const int64_t step;
  };
  struct ReverseIterator {
    explicit ReverseIterator(int64_t position, int64_t step = 0) : position(position), step(step) {}
    auto operator*() const -> int64_t { return position; }
    auto operator!=(const ReverseIterator& source) const -> bool { return step > 0 ? position > source.position : position < source.position; }
    auto operator++() -> ReverseIterator& { position -= step; return *this; }

   private:
    int64_t position;
    const int64_t step;
  };

  auto begin() const -> Iterator { return Iterator{origin, stride}; }
  auto end() const -> Iterator { return Iterator{target}; }

  auto rbegin() const -> ReverseIterator { return ReverseIterator{target - stride, stride}; }
  auto rend() const -> ReverseIterator { return ReverseIterator{origin - stride}; }

  int64_t origin;
  int64_t target;
  int64_t stride;
};

struct ReversedRange {
  auto begin() const -> Range::ReverseIterator { return base_range.rbegin(); }
  auto end() const -> Range::ReverseIterator { return base_range.rend(); }

  auto rbegin() const -> Range::Iterator { return base_range.begin(); }
  auto rend() const -> Range::Iterator { return base_range.end(); }

  Range base_range;
};

inline auto range(int64_t size) -> Range { return Range{0, size, 1}; }
inline auto range(int64_t offset, int64_t size) -> Range { return Range{offset, size, 1}; }
inline auto range(int64_t offset, int64_t size, int64_t step) -> Range { return Range{offset, size, step}; }

inline auto rrange(int64_t size) -> ReversedRange { return ReversedRange{range(size)}; }
inline auto rrange(int64_t offset, int64_t size) -> ReversedRange { return ReversedRange{range(offset, size)}; }
inline auto rrange(int64_t offset, int64_t size, int64_t step) -> ReversedRange { return ReversedRange{range(offset, size, step)}; }
}

#endif //RANGE_HPP_
