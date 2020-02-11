#pragma once

#include "format/rip.hpp"
#include "table.hpp"


namespace ripv2 {

namespace exchanging {

using namespace format;

struct RipPacket {

  bool is_response_;
  std::vector<table::RoutingTable::Entry> entries_;

  bool from_buffer(rip::PacketHeaderReader header_reader,
      uint32_t interface_index, Ipv4Address source_address) {
    if (!rip::PacketValidator{header_reader.ptr_,
        header_reader.entry_num_}()) {
      return false;
    }
    is_response_ = header_reader.read_command() != 1;
    auto entry_reader = header_reader.first_entry();
    for (size_t i=0; i<header_reader.entry_num_; ++i) {
      entries_.push_back({
        table::Ipv4Prefix {
          entry_reader.read_ip_address(),
          entry_reader.read_subnet_mask(),
        },
        entry_reader.read_metric() + 1,
        interface_index,
        source_address,
      });
      entry_reader = entry_reader.next();
    }
    return true;
  }

  size_t to_buffer(BigEndianBufferWriter &writer, size_t start) const {
    writer.put_u8(!is_response_?1:2);  // command (1)
    writer.put_u8(2);  // version (1)
    writer.put_u16(0);  // must be zero (2)
    size_t count = 0;
    for (size_t i=start; i<entries_.size()&&count<25; ++i, ++count) {
      writer.put_u16(!is_response_?0:2);
        // ^ Address Family Identifier (2)
      writer.put_u16(0);  // Route Tag (2)
      writer.put_u32(entries_[i].prefix.address_.data_);  // IP Address (4)
      writer.put_u32(entries_[i].prefix.mask_.data_);  // Subnet Mask (4)
      writer.put_u32(entries_[i].next_hop.data_);  // Next Hop (4)
      writer.put_u32(entries_[i].metric);  // Metric (4)
    }
    return count;
  }

  size_t to_buffer_with_ip_header(BigEndianBufferWriter &writer,
      Ipv4Address source_address, Ipv4Address destination_address,
      size_t start) const {
    ip::HeaderWriter ih_writer{writer.ptr_};
    writer.put_u8(4*16+5);  // Version, IHL
    writer.put_u8(0);  // Type of Service
    writer.put_u16(20*entries_.size()+32);  // Total Length
    writer.put_u32(0);  // Identification, Flags, Fragment Offset
    writer.put_u8(1);  // Time to Live
    writer.put_u8(17);  // Protocol
    writer.ptr_ += 2;  // Header Checksum
    writer.put_u32(source_address.data_);  // Source Address
    writer.put_u32(destination_address.data_);  // Destination Address
    writer.put_u16(520);  // UDP Source Port
    writer.put_u16(520);  // UDP Destination Port
    writer.put_u16(20*entries_.size()+12);  // UDP Length
    writer.put_u16(0);  // UDP Checksum
    size_t count = to_buffer(writer, start);
    ih_writer.write_header_checksum();
    return count;
  }
};

struct InputProcessor {

  table::RoutingTable *table_;

  void process_entry(table::RoutingTable::Entry entry,
      std::vector<table::RoutingTable::Entry> &changed) {
    for (auto &e : table_->entries_) {
      if (entry.prefix == e.prefix) {
        if (entry.metric < e.metric) {
          e = entry;
          changed.push_back(entry);
        }
        return;
      }
    }
    table_->entries_.push_back(entry);
    changed.push_back(entry);
  }

  std::vector<table::RoutingTable::Entry>
      process_response(const RipPacket &packet) {
    std::vector<table::RoutingTable::Entry> changed;
    for (auto e : packet.entries_) {
      process_entry(e, changed);
    }
    return changed;
  }
};

struct OutputGenerator {

  const table::RoutingTable *table_;

  RipPacket generate_whole_table_request() const {
    return { false, { { table::Ipv4Prefix{0, 0}, 16, 0, 0 } } };
  }

  RipPacket generate_unsolicited_response() const {
    return { true, table_->entries_ };
  }
};
}
}
