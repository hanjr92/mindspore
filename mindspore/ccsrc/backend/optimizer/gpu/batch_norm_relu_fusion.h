/**
 * Copyright 2020 Huawei Technologies Co., Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef MINDSPORE_CCSRC_BACKEND_OPTIMIZER_GPU_BATCH_NORM_RELU_FUSION_H_
#define MINDSPORE_CCSRC_BACKEND_OPTIMIZER_GPU_BATCH_NORM_RELU_FUSION_H_

#include <memory>
#include "backend/optimizer/common/optimizer.h"

namespace mindspore {
namespace opt {
class BatchNormReluFusion : public PatternProcessPass {
 public:
  explicit BatchNormReluFusion(bool multigraph = true) : PatternProcessPass("batch_norm_relu_fusion", multigraph) {
    x_ = std::make_shared<Var>();
    scale_ = std::make_shared<Var>();
    bias_ = std::make_shared<Var>();
    mean_ = std::make_shared<Var>();
    var_ = std::make_shared<Var>();
    index_ = std::make_shared<Var>();
  }
  ~BatchNormReluFusion() override = default;
  const BaseRef DefinePattern() const override;
  const AnfNodePtr Process(const FuncGraphPtr &, const AnfNodePtr &, const EquivPtr &) const override;

 private:
  VarPtr x_;
  VarPtr scale_;
  VarPtr bias_;
  VarPtr mean_;
  VarPtr var_;
  VarPtr index_;
};
}  // namespace opt
}  // namespace mindspore
#endif  // MINDSPORE_CCSRC_BACKEND_OPTIMIZER_GPU_BATCH_NORM_RELU_FUSION_H_
