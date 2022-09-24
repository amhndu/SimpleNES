#pragma once

#ifndef MAYBE_HPP_
#define MAYBE_HPP_

#include <cassert>

namespace sen {

struct nothing_t {};
static nothing_t nothing;
struct else_t {};

template<typename T>
struct maybe {
  inline maybe() = default;
  inline maybe(nothing_t) {}                              // NOLINT(google-explicit-constructor)
  inline maybe(const T& source) { operator=(source); }    // NOLINT(google-explicit-constructor)
  inline maybe(T&& source) { operator=(move(source)); }   // NOLINT(google-explicit-constructor)
  inline maybe(const maybe& source) { operator=(source); }
  inline maybe(maybe &&source) noexcept { operator=(move(source)); }
  inline ~maybe() { reset(); }

  inline auto operator=(nothing_t) -> maybe& { reset(); return *this; }
  inline auto operator=(const T& source) -> maybe& { reset(); _valid = true; new(&_value.t) T(source); return *this; }
  inline auto operator=(T&& source) -> maybe& { reset(); _valid = true; new(&_value.t) T(move(source)); return *this; }

  inline auto operator=(const maybe& source) -> maybe& {
    if(this == &source) return *this;
    reset();
    if((_valid = source._valid)) new(&_value.t) T(source.get());
    return *this;
  }

  inline auto operator=(maybe &&source) noexcept -> maybe & {
    if (this == &source) return *this;
    reset();
    if ((_valid = source._valid)) new(&_value.t) T(move(source.get()));
    return *this;
  }

  inline explicit operator bool() const { return _valid; }
  inline auto reset() -> void { if(_valid) { _value.t.~T(); _valid = false; } }
  inline auto data() -> T* { return _valid ? &_value.t : nullptr; }
  inline auto get() -> T& { assert(_valid); return _value.t; }

  inline auto data() const -> const T* { return ((maybe*)this)->data(); }
  inline auto get() const -> const T& { return ((maybe*)this)->get(); }
  inline auto operator->() -> T* { return data(); }
  inline auto operator->() const -> const T* { return data(); }
  inline auto operator*() -> T& { return get(); }
  inline auto operator*() const -> const T& { return get(); }
  inline auto operator()() -> T& { return get(); }
  inline auto operator()() const -> const T& { return get(); }
  inline auto operator()(const T& invalid) const -> const T& { return _valid ? get() : invalid; }

 private:
  union U {
    T t;
    U() = default;
    ~U() = default;
  } _value;
  bool _valid = false;
};

template<typename T>
struct maybe<T&> {
  inline maybe() : _value(nullptr) {}
  inline maybe(nothing_t) : _value(nullptr) {}                  // NOLINT(google-explicit-constructor)
  inline maybe(const T& source) : _value((T*)&source) {}        // NOLINT(google-explicit-constructor)
  inline maybe(const maybe& source) : _value(source._value) {}

  inline auto operator=(nothing_t) -> maybe& { _value = nullptr; return *this; }
  inline auto operator=(const T& source) -> maybe& { _value = (T*)&source; return *this; }
  inline auto operator=(const maybe& source) -> maybe& { _value = source._value; return *this; }

  inline explicit operator bool() const { return _value; }
  inline auto reset() -> void { _value = nullptr; }
  inline auto data() -> T* { return _value; }
  inline auto get() -> T& { assert(_value); return *_value; }

  inline auto data() const -> const T* { return ((maybe*)this)->data(); }
  inline auto get() const -> const T& { return ((maybe*)this)->get(); }
  inline auto operator->() -> T* { return data(); }
  inline auto operator->() const -> const T* { return data(); }
  inline auto operator*() -> T& { return get(); }
  inline auto operator*() const -> const T& { return get(); }
  inline auto operator()() -> T& { return get(); }
  inline auto operator()() const -> const T& { return get(); }
  inline auto operator()(const T& invalid) const -> const T& { return _value ? get() : invalid; }

 private:
  T* _value;
};

}

#endif //MAYBE_HPP_
