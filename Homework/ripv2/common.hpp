#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <vector>


namespace ripv2 {

using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using std::uint64_t;
using std::size_t;

static_assert(sizeof(uint8_t) == 1, "");

template<typename T> T unimplemented() { while (true) {} }

template<typename T> bool is_1x0x(T v) {
  static_assert(std::is_unsigned<T>::value, "");
  return (-v & ~v) == 0;
}

constexpr uint32_t quick_log2(uint32_t x) {
  constexpr uint32_t powof2_tb[] = {
    32, 0, 1, 26, 2, 23, 27, 32, 3, 16, 24, 30, 28, 11, 32, 13, 4, 7, 17, 32,
    25, 22, 31, 15, 29, 10, 12, 6, 32, 21, 14, 9, 5, 20, 8, 19, 18,
  };
  return powof2_tb[x%37];
}
}
