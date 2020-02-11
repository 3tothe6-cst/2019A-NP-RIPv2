#include "debug.hpp"
#include "exchanging.hpp"
#include "forwarding.hpp"
#include "hal.hpp"

using namespace ripv2;
using namespace ripv2::debug;
using namespace ripv2::environment;
using namespace ripv2::exchanging;
using namespace ripv2::forwarding;
using namespace ripv2::table;


constexpr int kCodeOnInitFailure = 101;
constexpr size_t kPacketBufferSize = 65536;
constexpr uint64_t kRegularResponsePeriod = 5000;

namespace {

RoutingTable generate_routing_table() {
  RoutingTable table;
  for (size_t i=0; i<kInterfaceNum; ++i) {
    auto address = kInterfaceAddresses[i];
    auto prefix = Ipv4Prefix::from_address_and_mask_length(address, 24);
    table.add({prefix, 1, static_cast<uint32_t>(i), 0});
  }
  return table;
}

void send_rip_packet(const RipPacket &packet, uint8_t *buffer,
    uint32_t interface_index, Ipv4Address destination_address) {
  for (size_t count=0; count<packet.entries_.size(); ) {
    BigEndianBufferWriter writer{buffer};
    size_t current = packet.to_buffer_with_ip_header(writer,
      kInterfaceAddresses[interface_index], destination_address, count);
    hal::send_ip_packet(buffer, 20*current+32,
      interface_index, destination_address);
    count += current;
  }
}

void generate_complete_response(const RoutingTable &table, uint8_t *buffer,
    uint32_t interface_index, Ipv4Address destination_address) {
  auto packet = OutputGenerator{&table}.generate_unsolicited_response();
  send_rip_packet(packet, buffer, interface_index, destination_address);
}

void generate_unsolicited_response(
    const RoutingTable &table, uint8_t *buffer) {
  auto packet = OutputGenerator{&table}.generate_unsolicited_response();
  for (size_t i=0; i<kInterfaceNum; ++i) {
    send_rip_packet(packet, buffer, i, kMulticastIpv4Address);
  }
}

void process_exchanging(RoutingTable &table,
    uint8_t *buffer, uint32_t interface_index) {
  RipPacket packet;
  auto reader = rip::PacketHeaderReader::from_ip_header_reader({buffer});
  Ipv4Address source_address = ip::HeaderReader{buffer}.read_source_address();
  if (packet.from_buffer(reader, interface_index, source_address)) {
    if (!packet.is_response_) {
      generate_complete_response(table, buffer,
        interface_index, source_address);
    } else {
      auto changed = InputProcessor{&table}.process_response(packet);
      for (size_t i=0; i<kInterfaceNum; ++i) {
        std::vector<RoutingTable::Entry> to_send;
        for (auto e : changed) {
          if (e.interface_index != i) {
            to_send.push_back(e);
          }
        }
        if (!to_send.empty()) {
          send_rip_packet({true, to_send}, buffer, i, kMulticastIpv4Address);
        }
      }
    }
  }
}

void process_forwarding(RoutingTable &table, uint8_t *buffer) {
  ip::HeaderReader reader{buffer};
  uint32_t interface_index;
  Ipv4Address next_hop;
  if (table.query(reader.read_destination_address(),
      interface_index, next_hop)) {
    if (next_hop.data_ == 0) {
      next_hop = reader.read_destination_address();
    }
    forward(ip::HeaderWriter{buffer});
    hal::send_ip_packet(buffer, reader.read_total_length(),
      interface_index, next_hop);
  }
}

void process_incoming_packet(RoutingTable &table, uint8_t *buffer) {
  uint32_t interface_index;
  int receive_result = hal::receive_ip_packet(buffer,
    kPacketBufferSize, interface_index);
  if (receive_result != 0) {
    return;
  }
  if (destination_is_me(ip::HeaderReader{buffer}.read_destination_address())) {
    process_exchanging(table, buffer, interface_index);
  } else {
    process_forwarding(table, buffer);
  }
}
}

int main() {
  spdlog::set_level(spdlog::level::debug);
    // ^ Not needed with future version of spdlog
  SPDLOG_INFO("RIPv2 Implementation in Modern C++ - started");
  if (!hal::init()) {
    return kCodeOnInitFailure;
  }
  auto table = generate_routing_table();
  uint64_t last_time = HAL_GetTicks();
  uint8_t buffer[kPacketBufferSize];
  while (true) {
    uint64_t current_time = HAL_GetTicks();
    if (current_time >= last_time + kRegularResponsePeriod) {
      last_time = current_time;
      print_routing_table_to_stderr(table);
      generate_unsolicited_response(table, buffer);
    } else {
      process_incoming_packet(table, buffer);
    }
  }
}
