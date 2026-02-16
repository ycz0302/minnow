#include "wrapping_integers.hh"
#include "debug.hh"

using namespace std;

uint64_t absolute_difference(uint64_t x, uint64_t y) {
  return x < y ? y - x : x - y;
}

Wrap32 Wrap32::wrap(uint64_t n, Wrap32 zero_point) {
  return Wrap32 {zero_point.raw_value_ + static_cast<uint32_t>(n)};
}

uint64_t Wrap32::unwrap(Wrap32 zero_point, uint64_t checkpoint) const {
  const uint32_t delta = this->raw_value_ - zero_point.raw_value_;
  const uint64_t mid = (checkpoint >> 32 << 32) + delta;
  const uint64_t mod = static_cast<uint64_t>(1) << 32;
  const uint64_t l = mid - mod;
  const uint64_t r = mid + mod;
  const uint64_t dl = absolute_difference(l, checkpoint);
  const uint64_t dr = absolute_difference(r, checkpoint);
  const uint64_t dmid = absolute_difference(mid, checkpoint);
  if (dl <= dr && dl <= dmid) {
    return l;
  }
  if (dr <= dl && dr <= dmid) {
    return r;
  }
  return mid;
}
