#pragma once

#ifndef SERIALIZER_HPP_
#define SERIALIZER_HPP_

#include <array>
#include <type_traits>
#include <string>
#include <cstring>

#include "range.hpp"

namespace sen {

class Serializer;
// How to judge whether a class has a member function through decltype() in C++11
template<typename T> struct HasSerialize {
 private:
  // template<typename U> static auto check(int) -> decltype(std::declval<U>().serialize(std::declval<serializer &>()), std::true_type()) { return {}; }
  template<typename U> static auto check(int) -> decltype(std::declval<U>().serialize(std::declval<Serializer &>()), std::true_type()) { return {}; }
  template<typename> static auto check(...) -> std::false_type { return {}; }

 public:
  static constexpr bool value = std::is_same<decltype(check<T>(0)), std::true_type>::value;
};

class Serializer {
 public:
  enum Mode { Save, Load, Size };

 private:
  uint     size_{0};
  uint     capacity_{0};
  uint8_t *data_{nullptr};
  Mode     mode_{Save};

 public:
  Serializer() = default;
  explicit Serializer(uint capacity) {
    mode_ = Save;
    data_ = new uint8_t[capacity]();
    size_ = 0;
    capacity_ = capacity;
  }
  Serializer(const uint8_t* data, uint capacity) {
    mode_ = Load;
    data_ = new uint8_t[capacity];
    size_ = 0;
    capacity_ = capacity;
    memcpy(data_, data, capacity);
  }
  Serializer(const Serializer& s) { operator=(s); }
  Serializer(Serializer&& s)  noexcept { operator=(std::move(s)); }
  ~Serializer() { delete[] data_; }

  auto operator=(const Serializer& s) -> Serializer& {
    if (&s == this) return *this;

    delete[] data_;

    mode_ = s.mode_;
    data_ = new uint8_t[s.capacity_];
    size_ = s.size_;
    capacity_ = s.capacity_;

    memcpy(data_, s.data_, s.capacity_);
    return *this;
  }
  auto operator=(Serializer&& s) noexcept -> Serializer& {
    if (&s == this) return *this;

    delete[] data_;
    mode_ = s.mode_;
    data_ = s.data_;
    size_ = s.size_;
    capacity_ = s.capacity_;

    s.data_ = nullptr;
    return *this;
  }

  explicit operator bool() const { return size_; }
  auto mode(Mode mode) -> void { mode_ = mode; size_ = 0; }
  auto mode() const -> Mode { return mode_; }
  auto data() const -> const uint8_t *{ return data_; }
  auto size() const -> uint { return size_; }
  auto capacity() const -> uint { return capacity_; }

  template<typename T> auto real(T &value) -> Serializer& {
    // this is rather dangerous, and not cross-platform safe;
    // but there is no standardized way to export FP-values
    // auto p = (uint8_t*)&value;
    // if(mode_ == Save) {
    //   for(uint n : range(type_size)) data_[size_++] = p[n];
    // } else if(mode_ == Load) {
    //   for(uint n : range(type_size)) p[n] = data_[size_++];
    // } else {
    //   _size += type_size;
    // }

    // TODO: Add support for long double
    if (mode_ == Save) {
      std::string str = std::to_string(value);
      for (uint n : range(int64_t(str.size()))) data_[size_++] = str[n];
    } else if (mode_ == Load) {
      std::string str;
      for (uint n : range(size_)) str.push_back(char(data_[n]));

      // TODO: Refactor that if-else into a function
      if (std::is_same<double, T>::value) {
        value = std::stod(str, nullptr);
      } else if (std::is_same<float, T>::value) {
        value = std::stof(str, nullptr);
      }
    } else {
      size_ += sizeof(T);
    }

    return *this;
  }
  template<typename T> auto boolean(T &value) -> Serializer& {
    if(mode_ == Save) {
      data_[size_++] = (bool)value;
    } else if(mode_ == Load) {
      value = (bool)data_[size_++];
    } else if(mode_ == Size) {
      size_ += 1;
    }
    return *this;
  }
  template<typename T> auto integer(T &value) -> Serializer& {
    enum : uint { type_size = std::is_same<bool, T>::value ? 1 : sizeof(T) };
    if(mode_ == Save) {
      uint64_t copy = value;
      for(uint n : range(type_size)) data_[size_++] = copy, copy >>= 8;
    } else if(mode_ == Load) {
      value = 0;
      for(uint n : range(type_size)) value |= (T)data_[size_++] << (n << 3);
    } else if(mode_ == Size) {
      size_ += type_size;
    }
    return *this;
  }
  template<typename T, int N> auto array(T (&array)[N]) -> Serializer& {
    for(uint n : range(N)) operator()(array[n]);
    return *this;
  }
  template<typename T> auto array(T array, uint size) -> Serializer& {
    for(uint n : range(size)) operator()(array[n]);
    return *this;
  }
  template<typename T, int N> auto array(std::array<T, N>& array) -> Serializer& {
    for(auto& value : array) operator()(value);
    return *this;
  }

  template<typename T> auto operator()(T &value, typename std::enable_if<HasSerialize<T>::value>::type * = 0) -> Serializer & { value.serialize(*this); return *this; }
  template<typename T> auto operator()(T &value, typename std::enable_if<std::is_integral<T>::value>::type * = 0) -> Serializer & { return integer(value); }
  template<typename T> auto operator()(T &value, typename std::enable_if<std::is_floating_point<T>::value>::type * = 0) -> Serializer & { return real(value); }
  template<typename T> auto operator()(T &value, typename std::enable_if<std::is_array<T>::value>::type* = 0) -> Serializer & { return array(value); }
  template<typename T> auto operator()(T &value, uint size, typename std::enable_if<std::is_pointer<T>::value>::type * = 0) -> Serializer & { return array(value, size); }
};

}

#endif //SERIALIZER_HPP_
