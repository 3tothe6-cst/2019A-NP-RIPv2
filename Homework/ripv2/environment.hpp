#pragma once

#include "format/common.hpp"


namespace ripv2 {

namespace environment {

using namespace format;

constexpr size_t kInterfaceNum = 2;
constexpr std::array<Ipv4Address,
    kInterfaceNum> kInterfaceAddresses = {
#if ROUTER_NUMBER == 1
  Ipv4Address::from_octets(192, 168, 1, 1),
  Ipv4Address::from_octets(192, 168, 3, 1),
#elif ROUTER_NUMBER == 3
  Ipv4Address::from_octets(192, 168, 4, 2),
  Ipv4Address::from_octets(192, 168, 5, 2),
#else
  Ipv4Address::from_octets(192, 168, 3, 2),
  Ipv4Address::from_octets(192, 168, 4, 1),
#endif
};

constexpr Ipv4Address kMulticastIpv4Address
  = Ipv4Address::from_octets(224, 0, 0, 9);
constexpr MacAddress kMulticastMacAddress
  = {0x01, 0x00, 0x5e, 0x00, 0x00, 0x09};

inline bool destination_is_me(Ipv4Address destination_address) {
  return destination_address == kMulticastIpv4Address
    || std::find(kInterfaceAddresses.cbegin(), kInterfaceAddresses.cend(),
      destination_address) != kInterfaceAddresses.cend();
}
}
}
