#include "tcp_receiver.hh"
#include "debug.hh"

using namespace std;

void TCPReceiver::receive(TCPSenderMessage message) {
  if (message.RST) {
    this->reassembler_.reader().set_error();
    return;
  }
  if (message.SYN) {
    this->zero_point = message.seqno;
    this->zero_point_tag = true;
  }
  if (!this->zero_point_tag) {
    return;
  }
  uint64_t first_index = message.seqno.unwrap(this->zero_point, this->writer().bytes_pushed());
  if (!message.SYN) {
    first_index--;
  }
  this->reassembler_.insert(first_index, message.payload, message.FIN);
}

TCPReceiverMessage TCPReceiver::send() const {
  TCPReceiverMessage res {};
  if (!this->zero_point_tag) {
    res.ackno = std::nullopt;
  }
  else {
    uint64_t ackno = this->writer().bytes_pushed();
    if (this->reassembler_.seq_len().has_value() && ackno == this->reassembler_.seq_len()) {
      ackno++;
    }
    ackno++;
    res.ackno = Wrap32::wrap(ackno, this->zero_point);
  }
  uint64_t cap = this->reassembler_.writer().available_capacity();
  if (cap > 65535) {
    cap = 65535;
  }
  res.window_size = static_cast<uint16_t>(cap);
  res.RST = this->reassembler_.reader().has_error();
  return res;
}
