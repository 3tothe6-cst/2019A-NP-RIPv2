#pragma once

#include <boost/endian/conversion.hpp>

#include "format/ip.hpp"
#include "environment.hpp"

#include "../../HAL/include/router_hal.h"


namespace ripv2 {

namespace hal {

using boost::endian::endian_reverse;
using namespace format;
using namespace environment;

inline bool init() {
  std::array<uint32_t, kInterfaceNum> addresses;
  for (size_t i=0; i<kInterfaceNum; ++i) {
    addresses[i] = endian_reverse(kInterfaceAddresses[i].data_);
  }
  int result = HAL_Init(0, addresses.data());
  if (result != 0) {
    SPDLOG_CRITICAL("HAL initialization failed (code: {})", result);
  } else {
    SPDLOG_DEBUG("HAL initialization done");
  }
  return result == 0;
}

inline bool send_ip_packet(const uint8_t *buffer, size_t length,
    uint32_t interface_index, Ipv4Address destination_address) {
  MacAddress destination_mac_address;
  if (destination_address == kMulticastIpv4Address) {
    destination_mac_address = kMulticastMacAddress;
  } else {
    constexpr char format_string[]
      = "Try to query MAC address of {} from interface {}...";
    SPDLOG_DEBUG(format_string, destination_address, interface_index);
    int result = HAL_ArpGetMacAddress(interface_index,
      endian_reverse(destination_address.data_),
        destination_mac_address.data_.data());
    if (result != 0) {
      SPDLOG_DEBUG("    ...failed (code: {})", result);
      return false;
    } else {
      SPDLOG_DEBUG("    ...done: {}", destination_mac_address);
    }
  }
  constexpr char format_string[]
    = "Try to send an IP packet of length {} to {} by interface {}...";
  SPDLOG_DEBUG(format_string, length,
    destination_mac_address, interface_index);
  int result = HAL_SendIPPacket(interface_index,
    const_cast<uint8_t*>(buffer), length,
      destination_mac_address.data_.data());
  if (result != 0) {
    SPDLOG_DEBUG("    ...failed (code: {})", result);
    return false;
  } else {
    SPDLOG_DEBUG("    ...done");
    return true;
  }
}

inline int receive_ip_packet(uint8_t *buffer,
    size_t capacity, uint32_t &interface_index) {
  MacAddress source, destination;
  int32_t interface_index_as_signed;
  int result = HAL_ReceiveIPPacket((1<<kInterfaceNum)-1,
    buffer, capacity, source.data_.data(), destination.data_.data(),
      1000, &interface_index_as_signed);
  if (result == 0) {
    return 2;
  }
  if (result < 0) {
    constexpr char format_string[]
      = "An error occured while receiving IP packet (code: {})";
    SPDLOG_DEBUG(format_string, result);
    return 1;
  }
  if (ip::HeaderReader{buffer}.read_total_length() != result
      || !ip::Validator{buffer}()) {
    constexpr char format_string[] = "Received a broken IP packet \
from interface {} (source: {}, destination: {})";
    SPDLOG_DEBUG(format_string,
      interface_index_as_signed, source, destination);
    return 1;
  }
  constexpr char format_string[] = "Received an IP packet of length {} \
from interface {} (source: {}, destination: {})";
  SPDLOG_DEBUG(format_string, result,
    interface_index_as_signed, source, destination);
  interface_index = interface_index_as_signed;
  return 0;
}
}
}
