#pragma once

#include <mutex>

#include "../../x_ml/src/xmodel.hh"
#include "../../xkv_core/src/lib.hh"

#include "./sample_trait.hh"

namespace xstore {

namespace xcache {

using namespace xstore::xkv;
using namespace xstore::xml;

/*!
  This file implements a traininer for training the second-layer submodel,
  defined in ../../x_ml/src/xmodel.hh
 */
template <typename KeyType>
struct XMLTrainer {
  // the trainer may be trained in a (background) thread
  std::mutex guard;

  // This model is responsible for predicting [start_key, end_key]
  KeyType start_key = KeyType::max();
  KeyType end_key = KeyType::min();

  /*! seqs which used to define whether this model need to retrain
    the model will train if and only if h_watermark > l_watermark
    FIXME: currently not handle cases when h_watermark/l_watermark overflows
  */
  usize l_watermark = 0;
  usize h_watermark = 0;

  /*********************/

  auto update_key(const KeyType &k) {
    std::lock_guard<std::mutex> lock(this->guard);
    this->start_key = std::min(this->start_key, k);
    this->end_key = std::max(this->end_key, k);

    // update the watermark to notify retrain
    if (!this->need_train()) {
      this->h_watermark += 1;
    }
  }

  auto need_train() -> bool { return this->h_watermark > this->l_watermark; }

  /*!
    core training method
    \note: assumption this method will only be called in a single-threaded
    context
  */
  template <class IT, template<typename> class S, template <typename>  class SubML>
  auto train(typename IT::KV &kv, S<KeyType> &s, update_func f = default_update_func)
      -> XSubModel<SubML, KeyType> {
    auto iter = IT::from(kv);
    return train_w_it<IT, S, SubML>(iter, kv, s, f);
  }

  template <class IT, template <typename> class S, template <typename>  class SubML>
  auto train_w_it(IT &iter, typename IT::KV &kv, S<KeyType> &s,
                  update_func f = default_update_func)
      -> XSubModel<SubML, KeyType> {
    // TODO: model should be parameterized
    XSubModel<SubML,KeyType> model;

    if (!this->need_train()) {
      return model;
    }
    // LOG(4) << "train model w : " << this->start_key << " :" << this->end_key;

    // take a snapshot of the start/end key
    KeyType s_key;
    KeyType e_key;
    {
      std::lock_guard<std::mutex> lock(this->guard);
      s_key = this->start_key;
      e_key = this->end_key;
      this->l_watermark += 1;
    }

    std::vector<KeyType> train_set;
    std::vector<u64> train_label;

    // base is used to algin the labels start from 0
    u64 base = 0; // legacy

    for (iter.seek(s_key, kv); iter.has_next(); iter.next()) {
      if (iter.cur_key() > e_key) {
        break;
      }
      s.add_to(iter.cur_key(), iter.opaque_val() - base, train_set,
               train_label);
    }
    s.finalize(train_set, train_label);

    // 2. train the model
    model.train(train_set, train_label, f);

    return model;
  }
};

} // namespace xcache
} // namespace xstore
