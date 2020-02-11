#pragma once

#ifndef NDEBUG
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#else
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO
#endif

#include <cassert>  // needed by spdlog

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include "table.hpp"


#define DEFINE_PARSE constexpr auto parse( \
  format_parse_context &ctx) { return ctx.begin(); }

template<> struct fmt::formatter<ripv2::format::Ipv4Address> {

  DEFINE_PARSE

  template<typename FormatContext> auto format(
      const ripv2::format::Ipv4Address &address, FormatContext &ctx) {
    auto octets = address.to_octets();
    uint32_t a = octets[0], b = octets[1], c = octets[2], d = octets[3];
    return format_to(ctx.out(), "{}.{}.{}.{}", a, b, c, d);
  }
};

template<> struct fmt::formatter<ripv2::format::MacAddress> {

  DEFINE_PARSE

  template<typename FormatContext> auto format(
      const ripv2::format::MacAddress &address, FormatContext &ctx) {
    auto &data = address.data_;
    uint32_t a = data[0], b = data[1], c = data[2],
      d = data[3], e = data[4], f = data[5];
    constexpr char format_string[]
      = "{:02x}:{:02x}:{:02x}:{:02x}:{:02x}:{:02x}";
    return format_to(ctx.out(), format_string, a, b, c, d, e, f);
  }
};

template<> struct fmt::formatter<ripv2::table::Ipv4Prefix> {

  DEFINE_PARSE

  template<typename FormatContext> auto format(
      const ripv2::table::Ipv4Prefix &prefix, FormatContext &ctx) {
    uint32_t mask = prefix.mask_.data_;
    if (ripv2::is_1x0x(mask)) {
      uint32_t length = 32 - ripv2::quick_log2(-mask);
      return format_to(ctx.out(), "{}/{}", prefix.address_, length);
    } else {
      constexpr char format_string[] = "Ipv4Prefix(addr={}, mask={})";
      return format_to(ctx.out(), format_string, prefix.address_, prefix.mask_);
    }
  }
};

template<> struct fmt::formatter<ripv2::table::RoutingTable::Entry> {

  DEFINE_PARSE

  template<typename FormatContext> auto format(
      const ripv2::table::RoutingTable::Entry &entry, FormatContext &ctx) {
    constexpr char format_string[]
      = "Entry(prefix={}, metric={}, if_index={}, next_hop={})";
    return format_to(ctx.out(), format_string, entry.prefix,
      entry.metric, entry.interface_index, entry.next_hop);
  }
};

namespace ripv2 {

namespace debug {

  using namespace format;

  inline void print_routing_table_to_stderr(const table::RoutingTable &table) {
    SPDLOG_INFO("-----BEGIN ROUTING TABLE-----");
    for (auto e : table.entries_) {
      SPDLOG_INFO("{};", e);
    }
    SPDLOG_INFO("-----END ROUTING TABLE-----");
  }
}
}

#undef DEFINE_PARSE
