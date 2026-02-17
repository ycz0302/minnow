#include "tcp_sender.hh"
#include "debug.hh"
#include "tcp_config.hh"
using namespace std;

// How many sequence numbers are outstanding?
uint64_t TCPSender::sequence_numbers_in_flight() const {
  return this->flight_count;
}

// How many consecutive retransmissions have happened?
uint64_t TCPSender::consecutive_retransmissions() const {
  return this->retran_count;
}

void TCPSender::push(const TransmitFunction& transmit) {
  while (true) {
    TCPSenderMessage res {};
    const bool add = (this->window == 0);
    this->window += add;

    res.SYN = !this->SYN_tag;
    res.FIN = false;
    res.RST = this->reader().has_error();
    res.payload = "";
    res.seqno = this->isn_;
    const auto str = this->input_.reader().peek();
    uint64_t len = min(TCPConfig::MAX_PAYLOAD_SIZE, str.size());
    len = min(len, this->window - this->sequence_numbers_in_flight() - res.sequence_length());
    res.payload = str.substr(0, len);
    res.seqno = Wrap32::wrap(this->abs_seqno(), this->isn_);
    this->input_.reader().pop(len);
    if (!this->FIN_tag && this->input_.reader().is_finished() && res.sequence_length() + 1 + this->sequence_numbers_in_flight() <= this->window) {
      res.FIN = true;
      this->FIN_tag = true;
    }
    this->SYN_tag = true;

    this->window -= add;

    if (res.sequence_length() > 0) {
      this->q.push(res);
      this->flight_count += res.sequence_length();
      transmit(res);
    }
    else {
      break;
    }
  }
}

TCPSenderMessage TCPSender::make_empty_message() const {
  TCPSenderMessage res {};
  res.SYN = res.FIN = false;
  res.payload = "";
  res.seqno = Wrap32::wrap(this->abs_seqno(), this->isn_);
  res.RST = this->reader().has_error();
  return res;
}

void TCPSender::receive(const TCPReceiverMessage& msg) {
  this->window = msg.window_size;
  if (msg.RST) {
    this->reader().set_error();
  }
  if (!msg.ackno.has_value()) {
    return;
  }
  while (!this->q.empty()) {
    const uint32_t p1 = q.front().seqno.sub(this->isn_);
    const uint32_t p2 = msg.ackno.value().sub(this->isn_);
    if (p2 > this->abs_seqno()) {
      break;
    }
    if (p1 + this->q.front().sequence_length() <= p2) {
      this->flight_count -= this->q.front().sequence_length();
      this->q.pop();
      this->timer = 0;
      this->retran_count = 0;
      this->RTO = this->initial_RTO_ms_;
    }
    else {
      break;
    }
  }
}

void TCPSender::tick(uint64_t ms_since_last_tick, const TransmitFunction& transmit) {
  if (q.empty()) {
    this->timer = 0;
    this->retran_count = 0;
    this->RTO = this->initial_RTO_ms_;
    return;
  }
  this->timer += ms_since_last_tick;
  if (this->timer >= this->RTO) {
    transmit(q.front());
    if (this->window != 0) {
      this->retran_count++;
      this->RTO <<= 1;
    }
    this->timer = 0;
  }
}
