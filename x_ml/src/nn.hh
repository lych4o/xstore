// TODO: this is legacy, should switch to KeyType
#pragma once

#include "../deps/tiny-dnn/tiny_dnn/tiny_dnn.h"
#include "./ml_trait.hh"

namespace xstore {

namespace xml {

using namespace tiny_dnn;
using namespace tiny_dnn::activation;
// using namespace tiny_dnn::layers;

struct NN : MLTrait<NN> {
  network<sequential> net;

  /*!
    2 means that we convert a u64 to 2 float
   */

  //using act_type = ::tiny_dnn::activation::tanh;
  using act_type = ::tiny_dnn::activation::leaky_relu;
  explicit NN(const u64 &layout) : net(make_mlp<act_type>({2 * 1, layout, 1})) {}

  auto predict_impl(const u64 &key) -> double {
    auto res = net.predict(this->key_to_vec(key))[0];
    //LOG(4) << "predict key: " << key << " " << res;
    return res;
  }

  inline auto key_to_vec(const u64 &key) -> vec_t {
    //const u64 mask = std::numeric_limits<u32>::max();
    const u64 mask = 0xffffffffu;
    auto key_vec =
        vec_t({static_cast<float>(key & mask), static_cast<float>(key >> 32)});
    LOG(0) << "convert key: " << key << " to " << key_vec[0] << ","
           << key_vec[1];

    return key_vec;
  }

  void train_impl(std::vector<u64> &train_data, std::vector<u64> &train_label,
                  int step = 1) {

    std::vector<vec_t> real_train_set;
    std::vector<vec_t> labels;
    for (uint i = 0; i < train_data.size(); ++i) {
      real_train_set.push_back(this->key_to_vec(train_data[i]));
      labels.push_back(vec_t({static_cast<float>(train_label[i])}));
      //LOG(4) << "train key: " << real_train_set[i][0] << " "
      //<< real_train_set[i][1];
    }

    //this->net.weight_init(weight_init::constant(0.1));
    //this->net.bias_init(weight_init::constant(0.5));

    using opt = adagrad;
    //using opt = adam;
    //using opt = nesterov_momentum;
    // adagrad optimizer;
    // nesterov_momentum optimizer;

    opt optimizer;
    // train 120 epochs
    auto ret = this->net.train<mse, opt>(
        optimizer, real_train_set, labels, real_train_set.size(), 200000);
    ASSERT(ret) << "train error!";
    LOG(0) << "Train done using training-set:" << real_train_set.size();
    r2::compile_fence();
  }

  auto serialize_impl() -> std::string {
    ASSERT(false) << "not implemented";
    return "";
  }

  void from_serialize_impl(const std::string &data) {
    ASSERT(false) << "not implemented";
  }
};
} // namespace xml
} // namespace xstore
