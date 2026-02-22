#include <iostream>

#include "arp_message.hh"
#include "debug.hh"
#include "ethernet_frame.hh"
#include "exception.hh"
#include "helpers.hh"
#include "network_interface.hh"

using namespace std;

//! \param[in] ethernet_address Ethernet (what ARP calls "hardware") address of the interface
//! \param[in] ip_address IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface( string_view name,
                                    shared_ptr<OutputPort> port,
                                    const EthernetAddress& ethernet_address,
                                    const Address& ip_address )
  : name_( name )
  , port_( notnull( "OutputPort", move( port ) ) )
  , ethernet_address_( ethernet_address )
  , ip_address_( ip_address )
{
  cerr << "DEBUG: Network interface has Ethernet address " << to_string( ethernet_address_ ) << " and IP address "
       << ip_address.ip() << "\n";
}

//! \param[in] dgram the IPv4 datagram to be sent
//! \param[in] next_hop the IP address of the interface to send it to (typically a router or default gateway, but
//! may also be another host if directly connected to the same network as the destination) Note: the Address type
//! can be converted to a uint32_t (raw 32-bit IP address) by using the Address::ipv4_numeric() method.
void NetworkInterface::send_datagram(InternetDatagram dgram, const Address& next_hop) {
  const uint32_t target_ip = next_hop.ipv4_numeric();
  // Check if we already know the MAC for this IP
  for (auto &i : this->ip_mac_table) {
    if (i.first.ip == target_ip) {
      EthernetFrame frame;
      frame.header.src = this->ethernet_address_;
      frame.header.dst = i.first.mac;
      frame.header.type = EthernetHeader::TYPE_IPv4;
      frame.payload = serialize(dgram);
      this->transmit(frame);
      return;
    }
  }
  // Queue the datagram for later
  this->pending_datagrams.emplace_back(dgram, target_ip);
  // Check if we already have a pending ARP request for this IP
  bool transmit_arp = true;
  for (auto &i : this->arp) {
    if (i.first == target_ip) {
      transmit_arp = false;
    }
  }
  if (transmit_arp) {
    ARPMessage arpmsg;
    arpmsg.opcode = ARPMessage::OPCODE_REQUEST;
    arpmsg.sender_ethernet_address = this->ethernet_address_;
    arpmsg.sender_ip_address = this->ip_address_.ipv4_numeric();
    arpmsg.target_ethernet_address = {};
    arpmsg.target_ip_address = target_ip;

    EthernetFrame frame;
    frame.header.src = this->ethernet_address_;
    frame.header.dst = ETHERNET_BROADCAST;
    frame.header.type = EthernetHeader::TYPE_ARP;
    frame.payload = serialize(arpmsg);
    this->transmit(frame);
    this->arp.emplace_back(target_ip, 5000);
  }
}

//! \param[in] frame the incoming Ethernet frame
void NetworkInterface::recv_frame(EthernetFrame frame) {
  if (frame.header.type == EthernetHeader::TYPE_IPv4) {
    if (frame.header.dst == this->ethernet_address_) {
      InternetDatagram dgram;
      if (parse(dgram, frame.payload)) {
        this->datagrams_received_.push(dgram);
      }
    }
    return;
  }
  // ARP message
  if (frame.header.dst != ETHERNET_BROADCAST && frame.header.dst != this->ethernet_address_) {
    return;
  }
  ARPMessage arpmsg;
  if (parse(arpmsg, frame.payload)) {
    this->ip_mac_table.emplace_back(IP_MAC_NODE {arpmsg.sender_ip_address, arpmsg.sender_ethernet_address}, 30000);
    // Send any queued datagrams for this IP
    auto it = this->pending_datagrams.begin();
    while (it != this->pending_datagrams.end()) {
      if (it->second == arpmsg.sender_ip_address) {
        EthernetFrame out;
        out.header.src = this->ethernet_address_;
        out.header.dst = arpmsg.sender_ethernet_address;
        out.header.type = EthernetHeader::TYPE_IPv4;
        out.payload = serialize(it->first);
        this->transmit(out);
        it = this->pending_datagrams.erase(it);
      } else {
        ++it;
      }
    }
    // If it's an ARP request targeting our IP, send a reply
    if (arpmsg.opcode == ARPMessage::OPCODE_REQUEST && arpmsg.target_ip_address == this->ip_address_.ipv4_numeric()) {
      ARPMessage reply;
      reply.opcode = ARPMessage::OPCODE_REPLY;
      reply.sender_ethernet_address = this->ethernet_address_;
      reply.sender_ip_address = this->ip_address_.ipv4_numeric();
      reply.target_ethernet_address = arpmsg.sender_ethernet_address;
      reply.target_ip_address = arpmsg.sender_ip_address;

      EthernetFrame res;
      res.header.src = this->ethernet_address_;
      res.header.dst = arpmsg.sender_ethernet_address;
      res.header.type = EthernetHeader::TYPE_ARP;
      res.payload = serialize(reply);
      this->transmit(res);
    }
  }
  
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick(const size_t ms_since_last_tick) {
  for (auto &i : this->arp) {
    i.second -= ms_since_last_tick;
  }
  for (auto &i : this->ip_mac_table) {
    i.second -= ms_since_last_tick;
  }
  while (!this->arp.empty() && this->arp.front().second <= 0) {
    uint32_t expired_ip = this->arp.front().first;
    // Drop any pending datagrams for this expired ARP request
    auto it = this->pending_datagrams.begin();
    while (it != this->pending_datagrams.end()) {
      if (it->second == expired_ip) {
        it = this->pending_datagrams.erase(it);
      } else {
        ++it;
      }
    }
    this->arp.pop_front();
  }
  while (!this->ip_mac_table.empty() && this->ip_mac_table.front().second <= 0) {
    this->ip_mac_table.pop_front();
  }
}
