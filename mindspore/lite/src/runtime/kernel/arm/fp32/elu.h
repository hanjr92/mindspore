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

#ifndef MINDSPORE_LITE_SRC_RUNTIME_KERNEL_ARM_FP32_ELU_H_
#define MINDSPORE_LITE_SRC_RUNTIME_KERNEL_ARM_FP32_ELU_H_

#include <vector>
#include "src/lite_kernel.h"
#include "src/runtime/kernel/arm/nnacl/fp32/elu.h"

namespace mindspore::kernel {
class EluCPUKernel : public LiteKernel {
 public:
  explicit EluCPUKernel(OpParameter *parameter, const std::vector<lite::tensor::Tensor *> &inputs,
                        const std::vector<lite::tensor::Tensor *> &outputs, const lite::Context *ctx,
                        const mindspore::lite::PrimitiveC *primitive)
      : LiteKernel(parameter, inputs, outputs, ctx, primitive), ctx_(ctx), thread_count_(ctx->thread_num_) {}
  ~EluCPUKernel() override{};

  int Init() override;
  int ReSize() override;
  int Run() override;
  int DoExcute(int task_id);

 protected:
  int thread_count_;
  const lite::Context *ctx_;
  EluParameter *elu_parameter_;

 private:
  float *input_addr;
  float *output_addr;
};
}  // namespace mindspore::kernel

#endif  // MINDSPORE_LITE_SRC_RUNTIME_KERNEL_ARM_FP32_ELU_H