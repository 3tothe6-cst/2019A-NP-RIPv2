#pragma once

#include "ip.hpp"


namespace ripv2 {

namespace format {

namespace rip {

struct PacketEntryReader {

  const uint8_t *ptr_;

  uint16_t read_address_family_identifier() const {
    return BigEndianBufferReader{ptr_}.get_u16();
  }

  uint16_t read_route_tag() const {
    return BigEndianBufferReader{ptr_+2}.get_u16();
  }

  Ipv4Address read_ip_address() const {
    return { BigEndianBufferReader{ptr_+4}.get_u32() };
  }

  Ipv4Address read_subnet_mask() const {
    return { BigEndianBufferReader{ptr_+8}.get_u32() };
  }

  Ipv4Address read_next_hop() const {
    return { BigEndianBufferReader{ptr_+12}.get_u32() };
  }

  uint32_t read_metric() const {
    return BigEndianBufferReader{ptr_+16}.get_u32();
  }

  PacketEntryReader next() const {
    return { ptr_ + 20 };
  }
};

struct PacketEntryValidator {

  const uint8_t *ptr_;

  bool operator()(bool is_response) const {
    PacketEntryReader reader{ptr_};
    if ((!is_response && reader.read_address_family_identifier() != 0)
        || (is_response && reader.read_address_family_identifier() != 2)) {
      return false;
    }
    if (reader.read_route_tag() != 0) {
      return false;
    }
    if (!is_1x0x(reader.read_subnet_mask().data_)) {
      return false;
    }
    if (reader.read_metric() < 1 || reader.read_metric() > 16) {
      return false;
    }
    return true;
  }
};

struct PacketHeaderReader {

  const uint8_t *ptr_;
  size_t entry_num_;

  static PacketHeaderReader from_ip_header_reader(ip::HeaderReader reader) {
    size_t total_length = reader.read_total_length();
    return { reader.ptr_ + 28, (total_length - 32) / 20 };
  }

  uint8_t read_command() const {
    return BigEndianBufferReader{ptr_}.get_u8();
  }

  uint8_t read_version() const {
    return BigEndianBufferReader{ptr_+1}.get_u8();
  }

  uint16_t read_zero_after_version() const {
    return BigEndianBufferReader{ptr_+2}.get_u16();
  }

  PacketEntryReader first_entry() const {
    return { ptr_ + 4 };
  }
};

struct PacketValidator {

  const uint8_t *ptr_;
  size_t entry_num_;

  bool operator()() const {
    PacketHeaderReader header_reader{ptr_, entry_num_};
    uint8_t command = header_reader.read_command();
    if (command != 1 && command != 2) {
      return false;
    }
    if (header_reader.read_version() != 2) {
      return false;
    }
    if (header_reader.read_zero_after_version() != 0) {
      return false;
    }
    auto entry_reader = header_reader.first_entry();
    for (size_t i=0; i<entry_num_; ++i) {
      if (!PacketEntryValidator{entry_reader.ptr_}(command!=1)) {
        return false;
      }
      entry_reader = entry_reader.next();
    }
    return true;
  }
};
}
}
}
