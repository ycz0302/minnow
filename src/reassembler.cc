#include "reassembler.hh"
#include "debug.hh"

using namespace std;

void Reassembler::insert(uint64_t first_index, string data, bool is_last_substring) {
  if (is_last_substring) {
    last_tag = true;
    last_pos = first_index + data.size();
  }
  if (first_index >= this->end()) {
    return;
  }
  if (first_index + data.size() > this->end()) {
    data = data.substr(0, this->end() - first_index);
  }
  uint64_t need = 0;
  if (first_index + data.size() > this->head) {
    need = first_index + data.size() - this->head;
  }
  if (need > this->dq.size()) {
    dq.resize(need, make_pair('?', false));
  }
  for (uint64_t i = 0; i < data.size(); i++) {
    if (this->head > first_index + i) {
      continue;
    }
    const uint64_t pos = first_index + i - this->head;
    if (!this->dq[pos].second) {
      this->bytes_pending++;
    }
    this->dq[pos] = make_pair(data[i], true);
  }
  while(!dq.empty()) {
    auto [c, ok] = dq.front();
    if (!ok) {
      break;
    }
    this->bytes_pending--;
    this->output_.writer().push({c});
    this->head++;
    dq.pop_front();
  }
  if (last_tag && this->next_pos() == last_pos) {
    this->output_.writer().close();
  }
}

// How many bytes are stored in the Reassembler itself?
// This function is for testing only; don't add extra state to support it.
uint64_t Reassembler::count_bytes_pending() const {
  return this->bytes_pending;
}
