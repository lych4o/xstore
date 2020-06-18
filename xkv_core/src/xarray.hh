#pragma once

#include "../../deps/r2/src/common.hh"
#include "../../xcomm/src/atomic_rw/wrapper_type.hh"

// Memblock, which abstract away a raw pointer
#include "../../deps/r2/src/mem_block.hh"

/*!
  XArray provides a sorted array as the KV
 */
namespace xstore {

namespace xkv {

using namespace r2;
using namespace ::xstore::xcomm::rw;

template <typename V> struct XArray {

  using VType = WrappedType<V>;

  MemBlock key_array;
  MemBlock val_array;

  // unsafe pointer
  // these two pointers would point to the address in key_array and val_array
  u64 *key_ptr = nullptr;
  WrappedType<V> *val_ptr = nullptr;

  usize size = 0;

  XArray(const MemBlock &key_mem, const MemBlock &val_mem)
      : key_array(key_mem), val_array(val_mem),
        key_ptr(reinterpret_cast<u64 *>(key_mem.mem_ptr)),
        val_ptr(reinterpret_cast<WrappedType<V> *>(val_mem.mem_ptr)) {}

  /*!
    \ret true if insertion success, false if failure
    insertion could be reject due to two reasons:
    1. there are no avaliable memory, either because key_mem is out of space or
    val_mem is out of space
    2. the k is smaller than the current key, because the array must be sorted
   */
  auto insert(const u64 &k, const V &v) -> bool {
    if (size != 0) {
      // check prev key
      if (this->keys_at(size - 1).value() >= k) {
        return false;
      }
    }
    // check whether there is free memory
    if ((this->size + 1) * sizeof(u64) > this->key_array.sz) {
      return false;
    }

    if ((this->size + 1) * sizeof(WrappedType<V>) > this->val_array.sz) {
      return false;
    }

    key_ptr[this->size] = k;
    val_ptr[this->size].reset(v);
    this->size += 1;

    return true;
  }

  auto keys_at(const int &idx) -> Option<u64> {
    if (likely(idx >= 0 && idx < size)) {
      return key_ptr[idx];
    }
    return {};
  }

  auto vals_at(const int &idx) -> Option<V> {
    if (likely(idx >= 0 && idx < size)) {
      return val_ptr[idx].get_payload();
    }
    return {};
  }
};

} // namespace xkv
} // namespace xstore