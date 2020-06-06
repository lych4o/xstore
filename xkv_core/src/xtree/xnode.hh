#pragma once

#include <limits>

#include "../../../xcomm/src/atomic_rw/wrapper_type.hh"
#include "./spin_lock.hh"

#include "./xkeys.hh"

namespace xstore {

namespace xkv {

namespace xtree {

using namespace ::xstore::xcomm::rw;

/*!
  - N: max keys in this node
  - V: the value type
  FIXME: what if the V is a pointer? usually its trivially to adapt,
  but how to program it in a nice format ?
 */
template <usize N, typename V> struct __attribute__((packed)) XNode {

  CompactSpinLock lock;

  using NodeK = XNodeKeys<N>;

  // keys
  WrappedType<NodeK> keys;

  // values
  WrappedType<V> values[N];

  // next pointer
  XNode<N, V> *next = nullptr;

  // methods
  XNode() = default;

  auto num_keys() -> usize { return keys.get_payload().num_keys(); }

  auto get_incarnation() -> u32 { return this->keys.get_payload().incarnation; }

  auto get_key(const int &idx) -> u64 {
    return keys.get_payload().get_key(idx);
  }

  auto get_value(const int &idx) -> Option<V> {
    // TODO: not check idx
    if (this->keys.get_payload().get_key(idx) != kInvalidKey) {
      return values[idx].get_payload();
    }
    return {};
  }

  /*!
    The start offset of keys
   */
  auto keys_start_offset() -> usize { return offsetof(XNode, keys); }

  /*!
    The start offset of values
   */
  auto value_start_offset() -> usize { return offsetof(XNode, values); }

  /*!
    Query method
   */
  auto search(const u64 &key) -> Option<u8> {
    return keys.get_payload().search(key);
  }

  auto insert(const u64 &key, const V &v, XNode<N, V> *candidate) -> bool {
    this->lock.lock();
    auto ret = this->raw_insert(key, v, candidate);
    this->lock.unlock();
    return ret;
  }

  void print() {
    for (uint i = 0;i < N; ++i) {
      if (this->get_key(i) != kInvalidKey) {
        LOG(4) << "keys: #" << i << " " << this->get_key(i);
      }
    }
  }

  /*!
    Core insert function
    raw means that we donot hold the lock.
    Warning: we assume that candidate is **exclusively owned** by this
    insertion thread, and we will use raw_insert on candidate \ret: true ->
    this node has splitted, the splitted node is stored in the candidate
   */
  auto raw_insert(const u64 &key, const V &v, XNode<N, V> *candidate)
      -> bool { // lock the node for atomicity
    bool ret = false;
    auto idx = this->keys.get_payload().add_key(key);
    if (idx) {
      // in-place update
      this->values[idx.value()].reset(v);
    } else {
      // split
      ASSERT(candidate != nullptr) << "split at the node:" << this << " " << candidate;
      /*
        Split is more tricky in XTree, because keys in this node can be
        un-sorted. Plan #1: select the medium key, and split Plan #2: first sort
        the keys, and then split Currently, we use plan #1 due to simplicity
       */

      // 1. increment the incarnation
      this->keys.get_payload().incarnation += 1;
      r2::compile_fence();

      // 2. move the pivot key
      auto pivot_key_idx = this->keys.get_payload().find_median_key().value();
      auto pivot_key = this->keys.get_payload().get_key(pivot_key_idx);

      // should not split
      auto r = candidate->raw_insert(
          pivot_key, this->values[pivot_key_idx].get_payload(), nullptr);
      ASSERT(r == false);
      this->keys.get_payload().clear(pivot_key_idx);

      // 3. then, move other keys
      for (uint i = 0; i < N; ++i) {
        auto k = this->get_key(i);
        if (k != kInvalidKey && k > pivot_key) {
          // insert
          auto r =
              candidate->raw_insert(k, this->values[i].get_payload(), nullptr);
          ASSERT(r == false);
          this->keys.get_payload().clear(i);
        }
      }

      r2::compile_fence();
      // 4. insert the newly inserted keys
      if (key > pivot_key) {
        candidate->raw_insert(key, v, nullptr);
      } else {
        this->raw_insert(key, v, nullptr); // should have a space
      }
      //ASSERT(this->num_keys() + candidate->num_keys() == N + 1);

      // re-set the next pointer
      candidate->next = this->next;
      this->next = candidate;

      ret = true; // notify the insertion process the splition
    }
    return ret;
  }
};
} // namespace xtree
} // namespace xkv
} // namespace xstore