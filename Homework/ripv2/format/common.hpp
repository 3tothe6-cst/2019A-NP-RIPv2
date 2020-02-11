#pragma once

#include "../common.hpp"


namespace ripv2 {

namespace format {

struct Ipv4Address {

  uint32_t data_;

  static constexpr Ipv4Address from_octets(
      uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
    uint32_t a_ = a, b_ = b, c_ = c, d_ = d;
    return { (a_ << 24) + (b_ << 16) + (c_ << 8) + d_ };
  }

  static constexpr Ipv4Address mask_from_length(uint32_t length) {
    uint32_t data = -(static_cast<uint64_t>(1) << (32 - length));
    return { data };
  }

  std::array<uint8_t, 4> to_octets() const {
    uint8_t a = data_ / 16777216, b = (data_ / 65536) % 256,
      c = (data_ / 256) % 256, d = data_ % 256;
    return {a, b, c, d};
  }

  constexpr bool operator==(const Ipv4Address &other) const {
    return this->data_ == other.data_;
  }

  constexpr bool operator!=(const Ipv4Address &other) const {
    return this->data_ != other.data_;
  }

  constexpr Ipv4Address operator&(const Ipv4Address &other) const {
    return { this->data_ & other.data_ };
  }
};

struct MacAddress {

  std::array<uint8_t, 6> data_;

  constexpr bool operator==(const MacAddress &other) const {
    return this->data_[0] == other.data_[0] && this->data_[1] == other.data_[1]
      && this->data_[2] == other.data_[2] && this->data_[3] == other.data_[3]
      && this->data_[4] == other.data_[4] && this->data_[5] == other.data_[5];
  }
};

struct BigEndianBufferReader {

  const uint8_t *ptr_;

  template<typename T> T get() {
    static_assert(std::is_unsigned<T>::value, "");
    T v = 0;
    for (size_t i=0; i<sizeof(T); ++i) {
      v = v * 256 + *ptr_++;
    }
    return v;
  }

  uint8_t get_u8() {
    return get<uint8_t>();
  }

  uint16_t get_u16() {
    return get<uint16_t>();
  }

  uint32_t get_u32() {
    return get<uint32_t>();
  }

  uint64_t get_u64() {
    return get<uint64_t>();
  }
};

struct BigEndianBufferWriter {

  uint8_t *ptr_;

  template<typename T> void put(T v) {
    static_assert(std::is_unsigned<T>::value, "");
    std::array<uint8_t, sizeof(T)> buffer;
    for (auto it=buffer.rbegin(); it!=buffer.rend(); ++it) {
      *it = v % 256;
      v /= 256;
    }
    for (auto c : buffer) {
      *ptr_++ = c;
    }
  }

  void put_u8(uint8_t v) {
    put<uint8_t>(v);
  }

  void put_u16(uint16_t v) {
    put<uint16_t>(v);
  }

  void put_u32(uint32_t v) {
    put<uint32_t>(v);
  }

  void put_u64(uint64_t v) {
    put<uint64_t>(v);
  }
};
}
}
