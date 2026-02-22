#include "router.hh"
#include "debug.hh"

#include <iostream>

using namespace std;

// route_prefix: The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
// prefix_length: For this route to be applicable, how many high-order (most-significant) bits of
//    the route_prefix will need to match the corresponding bits of the datagram's destination address?
// next_hop: The IP address of the next hop. Will be empty if the network is directly attached to the router (in
//    which case, the next hop address should be the datagram's final destination).
// interface_num: The index of the interface to send the datagram out on.
void Router::add_route(
  const uint32_t route_prefix,
  const uint8_t prefix_length,
  const optional<Address> next_hop,
  const size_t interface_num
) {
  routing_table_.push_back({route_prefix, prefix_length, next_hop, interface_num});
}

void Router::route_one_datagram( InternetDatagram& dgram )
{
  // Drop if TTL expired
  if ( dgram.header.ttl <= 1 ) {
    return;
  }
  dgram.header.ttl--;
  dgram.header.compute_checksum();

  // Longest-prefix match
  const Route* best = nullptr;
  for ( const auto& route : routing_table_ ) {
    const uint32_t mask = route.prefix_length == 0 ? 0 : ( 0xFFFFFFFF << ( 32 - route.prefix_length ) );
    if ( ( dgram.header.dst & mask ) == ( route.route_prefix & mask ) ) {
      if ( best == nullptr || route.prefix_length > best->prefix_length ) {
        best = &route;
      }
    }
  }

  if ( best == nullptr ) {
    return; // no match, drop
  }

  const Address next = best->next_hop.value_or( Address::from_ipv4_numeric( dgram.header.dst ) );
  interface( best->interface_num )->send_datagram( dgram, next );
}

// Go through all the interfaces, and route every incoming datagram to its proper outgoing interface.
void Router::route()
{
  for ( auto& iface : interfaces_ ) {
    auto& queue = iface->datagrams_received();
    while ( !queue.empty() ) {
      InternetDatagram dgram = std::move( queue.front() );
      queue.pop();
      route_one_datagram( dgram );
    }
  }
}
