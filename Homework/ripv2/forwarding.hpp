#pragma once

#include "format/ip.hpp"


namespace ripv2 {

namespace forwarding {

using namespace format;

inline void forward(ip::HeaderWriter writer) {
  uint8_t time_to_live = ip::HeaderReader{writer.ptr_}.read_time_to_live() - 1;
  writer.write_time_to_live(time_to_live);
  writer.write_header_checksum();
}
}
}
