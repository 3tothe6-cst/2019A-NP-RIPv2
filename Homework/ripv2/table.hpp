#pragma once

#include "format/common.hpp"


namespace ripv2 {

namespace table {

using namespace format;

struct Ipv4Prefix {

  Ipv4Address address_, mask_;

  static Ipv4Prefix from_address_and_mask_length(
      Ipv4Address address, uint32_t mask_length) {
    auto mask = Ipv4Address::mask_from_length(mask_length);
    return { address & mask, mask };
  }

  bool operator==(const Ipv4Prefix &other) const {
    return this->address_ == other.address_ && this->mask_ == other.mask_;
  }

  bool matched_by(Ipv4Address address) const {
    return (address & mask_) == address_;
  }
};

struct RoutingTable {

  struct Entry {
    Ipv4Prefix prefix;
    uint32_t metric;
    uint32_t interface_index;
    Ipv4Address next_hop;
  };

  std::vector<Entry> entries_;

  void add(Entry entry) {
    for (auto &e : entries_) {
      if (e.prefix == entry.prefix) {
        e = entry;
        return;
      }
    }
    entries_.push_back(entry);
  }

  void remove(Ipv4Prefix prefix) {
    for (auto it=entries_.cbegin(); it!=entries_.cend(); ++it) {
      if (it->prefix == prefix) {
        entries_.erase(it);
        return;
      }
    }
  }

  bool query(Ipv4Address address, uint32_t &interface_index,
      Ipv4Address &next_hop) const {
    const RoutingTable::Entry *found = nullptr;
    for (const auto &e : entries_) {
      if (e.prefix.matched_by(address) && (!found
          || e.prefix.mask_.data_ > found->prefix.mask_.data_)) {
        found = &e;
      }
    }
    if (!found) {
      return false;
    }
    interface_index = found->interface_index;
    next_hop = found->next_hop;
    return true;
  }
};
}
}
