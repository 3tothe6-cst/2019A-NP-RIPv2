#pragma once

#include "common.hpp"


namespace ripv2 {

namespace format {

namespace ip {

struct HeaderReader {

  const uint8_t *ptr_;

  uint8_t read_ihl() const {
    return ptr_[0] % 16;
  }

  uint16_t read_total_length() const {
    return BigEndianBufferReader{ptr_+2}.get_u16();
  }

  uint8_t read_time_to_live() const {
    return ptr_[8];
  }

  uint16_t read_header_checksum() const {
    return BigEndianBufferReader{ptr_+10}.get_u16();
  }

  Ipv4Address read_source_address() const {
    return { BigEndianBufferReader{ptr_+12}.get_u32() };
  }

  Ipv4Address read_destination_address() const {
    return { BigEndianBufferReader{ptr_+16}.get_u32() };
  }
};

struct Validator {

  const uint8_t *ptr_;

  uint16_t calculate_header_checksum() const {
    BigEndianBufferReader reader{ptr_};
    uint32_t checksum = 0;
    for (size_t i=0; i<5; ++i) {
      checksum += reader.get_u16();
    }
    reader.ptr_ += 2;
    size_t ihl = HeaderReader{ptr_}.read_ihl();
    for (size_t i=0; i<ihl*2-6; ++i) {
      checksum += reader.get_u16();
    }
    while (checksum >= 65536) {
      checksum = checksum / 65536 + checksum % 65536;
    }
    return 65535 - checksum;
  }

  bool operator()() const {
    uint16_t header_checksum = calculate_header_checksum();
    return header_checksum == HeaderReader{ptr_}.read_header_checksum();
  }
};

struct HeaderWriter {

  uint8_t *ptr_;

  void write_time_to_live(uint8_t time_to_live) const {
    ptr_[8] = time_to_live;
  }

  void write_header_checksum() const {
    uint16_t header_checksum = Validator{ptr_}.calculate_header_checksum();
    BigEndianBufferWriter{ptr_+10}.put_u16(header_checksum);
  }
};
}
}
}
