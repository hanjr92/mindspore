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

#include "src/runtime/kernel/arm/fp32/convolution_winograd.h"
#include "nnacl/fp32/conv.h"
#include "nnacl/pack.h"
#include "schema/model_generated.h"
#include "src/kernel_registry.h"
#include "include/errorcode.h"
#include "src/runtime/runtime_api.h"

using mindspore::kernel::KERNEL_ARCH::kCPU;
using mindspore::lite::KernelRegistrar;
using mindspore::lite::RET_ERROR;
using mindspore::lite::RET_MEMORY_FAILED;
using mindspore::lite::RET_OK;
using mindspore::schema::PrimitiveType_Conv2D;

namespace mindspore::kernel {
int ConvolutionWinogradCPUKernel::WinogradFilterTransform(const float *weight_data, float *matrix_g, float *matrix_gt,
                                                          int oc_block) {
  if (oc_block == 0) {
    MS_LOG(ERROR) << "Divide by zero";
    return RET_ERROR;
  }
  // original weight format : ohwi
  auto channel_in = conv_param_->input_channel_;
  auto channel_out = conv_param_->output_channel_;
  int oc_block_num = UP_DIV(channel_out, oc_block);
  int block_stride = channel_in * oc_block;
  int block_num_stride = block_stride * oc_block_num;

  // trans_filter = G*g*GT (g represents weight_data)
  // separate into two steps ===> tmp = (g * GT)T ===> trans = (tmp * GT)T   use same function:MatrixMultiplyWinograd
  auto tmp_data = reinterpret_cast<float *>(malloc(channel_in * input_unit_ * kernel_unit_ * sizeof(float)));
  if (tmp_data == nullptr) {
    MS_LOG(ERROR) << "malloc tmp_data failed.";
    return RET_MEMORY_FAILED;
  }
  auto trans_out_data = reinterpret_cast<float *>(malloc(channel_in * input_unit_ * input_unit_ * sizeof(float)));
  if (trans_out_data == nullptr) {
    free(tmp_data);
    MS_LOG(ERROR) << "malloc trans_out_data failed.";
    return RET_MEMORY_FAILED;
  }

#ifndef ENABLE_ARM
  auto tmp_data1 = reinterpret_cast<float *>(malloc(channel_in * input_unit_ * kernel_unit_ * sizeof(float)));
  if (tmp_data1 == nullptr) {
    free(tmp_data);
    free(trans_out_data);
    MS_LOG(ERROR) << "malloc tmp_data1 failed.";
    return RET_MEMORY_FAILED;
  }
  auto trans_out_data1 = reinterpret_cast<float *>(malloc(channel_in * input_unit_ * input_unit_ * sizeof(float)));
  if (trans_out_data1 == nullptr) {
    free(tmp_data);
    free(tmp_data1);
    free(trans_out_data);
    MS_LOG(ERROR) << "malloc trans_out_data1 failed.";
    return RET_MEMORY_FAILED;
  }
#endif

  int input_oz_offset = kernel_unit_ * kernel_unit_ * channel_in;
  for (int i = 0; i < channel_out; i++) {
    int out_c_block = i / oc_block;
    int out_c_res = i % oc_block;
    int output_oz_offset = out_c_block * block_stride + out_c_res;

#ifndef ENABLE_ARM
    // tmp_data = g * GT
    MatrixMultiplyWinograd(weight_data + i * input_oz_offset, matrix_gt, tmp_data, kernel_unit_, kernel_unit_,
                           input_unit_, channel_in, channel_in * 4);
    // tmp_data1 = (tmp_data)T
    PackHWCToWHC(tmp_data, tmp_data1, kernel_unit_, input_unit_, channel_in);
    // trans_out_data1 = tmp * GT
    MatrixMultiplyWinograd(tmp_data1, matrix_gt, trans_out_data1, input_unit_, kernel_unit_, input_unit_, channel_in,
                           channel_in * 4);
    // trans_out_data = (trans_out_data1)T
    PackHWCToWHC(trans_out_data1, trans_out_data, input_unit_, input_unit_, channel_in);
#else
    // tmp = (g * GT)T
    MatrixMultiplyWinograd(weight_data + i * input_oz_offset, matrix_gt, tmp_data, kernel_unit_, kernel_unit_,
                           input_unit_, channel_in, channel_in * 4);
    // trans = (tmp * GT)T
    MatrixMultiplyWinograd(tmp_data, matrix_gt, trans_out_data, input_unit_, kernel_unit_, input_unit_, channel_in,
                           channel_in * 4);
#endif

    int in_offset = 0;
    for (int j = 0; j < input_unit_; ++j) {
      for (int k = 0; k < input_unit_; ++k) {
        for (int c = 0; c < channel_in; ++c) {
          *(trans_weight_ + output_oz_offset + c * oc_block) = trans_out_data[in_offset + c];
        }
        in_offset += channel_in;
        output_oz_offset += block_num_stride;
      }
    }
  }
#ifndef ENABLE_ARM
  free(tmp_data1);
  free(trans_out_data1);
#endif
  free(tmp_data);
  free(trans_out_data);
  return RET_OK;
}

int ConvolutionWinogradCPUKernel::InitWeightBias() {
  auto filter_tensor = in_tensors_.at(kWeightIndex);
  int in_channel = filter_tensor->Channel();
  int out_channel = filter_tensor->Batch();
  conv_param_->input_channel_ = in_channel;
  conv_param_->output_channel_ = out_channel;

  int oc4 = UP_DIV(out_channel, C4NUM);
  const int oc_block = C8NUM;
  int oc_block_num = UP_DIV(out_channel, C8NUM);

  // set data
  auto trans_matrix_data_size = input_unit_ * input_unit_ * in_channel * oc_block_num * oc_block * sizeof(float);
  trans_weight_ = reinterpret_cast<float *>(malloc(trans_matrix_data_size));
  if (trans_weight_ == nullptr) {
    MS_LOG(ERROR) << "malloc matrix_buffer failed.";
    return RET_MEMORY_FAILED;
  }
  memset(trans_weight_, 0, trans_matrix_data_size);

  float matrix_g[64];
  float matrix_gt[64];
  float matrix_a[64];
  float matrix_at[64];
  float matrix_b[64];
  float matrix_bt[64];
  float coef = 1.0f;
  if (input_unit_ == 8) {
    coef = 0.5f;
  }
  auto ret =
    CookToomFilter(matrix_a, matrix_at, matrix_b, matrix_bt, matrix_g, matrix_gt, coef, output_unit_, kernel_unit_);
  if (ret != RET_OK) {
    MS_LOG(ERROR) << "get matrix g from CookToomFilter failed.";
    return ret;
  }
  auto weight_data = reinterpret_cast<float *>(filter_tensor->MutableData());
  ret = WinogradFilterTransform(weight_data, matrix_g, matrix_gt, oc_block);
  if (ret != RET_OK) {
    MS_LOG(ERROR) << "winograd filter transfrom failed.";
    return ret;
  }

  // init bias
  size_t new_bias_size = oc4 * C4NUM * sizeof(float);
  bias_data_ = reinterpret_cast<float *>(malloc(new_bias_size));
  if (bias_data_ == nullptr) {
    MS_LOG(ERROR) << "malloc bias_data_ failed.";
    return RET_MEMORY_FAILED;
  }
  memset(bias_data_, 0, new_bias_size);
  if (in_tensors_.size() == kInputSize2) {
    auto ori_bias_addr = reinterpret_cast<float *>(in_tensors_.at(kBiasIndex)->MutableData());
    memcpy(bias_data_, ori_bias_addr, out_channel * sizeof(float));
  } else {
    MS_ASSERT(in_tensors_.size() == kInputSize1);
  }
  return RET_OK;
}

int ConvolutionWinogradCPUKernel::InitTmpBuffer() {
  int channel_out = conv_param_->output_channel_;
  int oc8 = UP_DIV(channel_out, C8NUM);
#ifdef ENABLE_ARM32
  int tile_num = 4;
#else
  int tile_num = 12;
#endif
  MS_ASSERT(ctx_->allocator != nullptr);

  size_t tile_buffer_size =
    thread_count_ * tile_num * input_unit_ * input_unit_ * conv_param_->input_channel_ * sizeof(float);
  trans_input_ = reinterpret_cast<float *>(ctx_->allocator->Malloc(tile_buffer_size));
  if (trans_input_ == nullptr) {
    MS_LOG(ERROR) << "malloc trans_input_ failed.";
    return RET_MEMORY_FAILED;
  }

  gemm_out_ = reinterpret_cast<float *>(
    ctx_->allocator->Malloc(thread_count_ * tile_num * input_unit_ * input_unit_ * oc8 * C8NUM * sizeof(float)));
  if (gemm_out_ == nullptr) {
    MS_LOG(ERROR) << "malloc gemm_out_ failed.";
    return RET_ERROR;
  }

  tmp_data_ = reinterpret_cast<float *>(
    ctx_->allocator->Malloc(thread_count_ * C4NUM * input_unit_ * input_unit_ * sizeof(float)));
  if (tmp_data_ == nullptr) {
    MS_LOG(ERROR) << "malloc tmp_data_ failed.";
    return RET_MEMORY_FAILED;
  }

  col_buffer_ = reinterpret_cast<float *>(
    ctx_->allocator->Malloc(thread_count_ * tile_num * conv_param_->input_channel_ * sizeof(float)));
  if (col_buffer_ == nullptr) {
    MS_LOG(ERROR) << "malloc col_buffer_ failed.";
    return RET_ERROR;
  }

  tmp_buffer_address_list_[0] = trans_input_;
  tmp_buffer_address_list_[1] = gemm_out_;
  tmp_buffer_address_list_[2] = tmp_data_;
  tmp_buffer_address_list_[3] = col_buffer_;
  return RET_OK;
}

int ConvolutionWinogradCPUKernel::ConfigInputOutput() {
  in_func_ = GetInputTransFunc(input_unit_);
  if (in_func_ == nullptr) {
    MS_LOG(ERROR) << "in_func_ is null.";
    return RET_ERROR;
  }
  out_func_ = GetOutputTransFunc(input_unit_, output_unit_, conv_param_->act_type_);
  if (out_func_ == nullptr) {
    MS_LOG(ERROR) << "out_func_ is null.";
    return RET_ERROR;
  }
  return RET_OK;
}

int ConvolutionWinogradCPUKernel::Init() {
  kernel_unit_ = conv_param_->kernel_h_;
  input_unit_ = output_unit_ + kernel_unit_ - 1;
  conv_param_->input_unit_ = input_unit_;
  conv_param_->output_unit_ = output_unit_;
  auto ret = InitWeightBias();
  if (ret != RET_OK) {
    MS_LOG(ERROR) << "Init weight bias failed.";
    return RET_ERROR;
  }
  if (!InferShapeDone()) {
    return RET_OK;
  }
  return ReSize();
}

int ConvolutionWinogradCPUKernel::ReSize() {
  auto ret = ConvolutionBaseCPUKernel::CheckResizeValid();
  if (ret != RET_OK) {
    MS_LOG(ERROR) << "Resize is invalid.";
    return ret;
  }

  ret = ConvolutionBaseCPUKernel::Init();
  if (ret != RET_OK) {
    MS_LOG(ERROR) << "ConvolutionBase init failed.";
    return RET_ERROR;
  }

  kernel_unit_ = conv_param_->kernel_h_;
  input_unit_ = output_unit_ + kernel_unit_ - 1;
  conv_param_->input_unit_ = input_unit_;
  conv_param_->output_unit_ = output_unit_;

  ret = ConfigInputOutput();
  if (ret != RET_OK) {
    MS_LOG(ERROR) << "ConfigInputOutput failed.";
    return RET_ERROR;
  }
  return RET_OK;
}

int ConvolutionWinogradCPUKernel::RunImpl(int task_id) {
  auto input_tensor = in_tensors_.at(kInputIndex);
  auto ori_input_data = reinterpret_cast<float *>(input_tensor->MutableData());
  auto output_data = reinterpret_cast<float *>(out_tensors_.front()->MutableData());
  ConvWinogardFp32(ori_input_data, trans_weight_, reinterpret_cast<const float *>(bias_data_), output_data,
                   tmp_buffer_address_list_, task_id, conv_param_, in_func_, out_func_);
  return RET_OK;
}

int ConvolutionWinogradImpl(void *cdata, int task_id) {
  auto conv = reinterpret_cast<ConvolutionWinogradCPUKernel *>(cdata);
  auto error_code = conv->RunImpl(task_id);
  if (error_code != RET_OK) {
    MS_LOG(ERROR) << "ConvolutionWinograd Run error task_id[" << task_id << "] error_code[" << error_code << "]";
    return RET_ERROR;
  }
  return RET_OK;
}

int ConvolutionWinogradCPUKernel::Run() {
  auto prepare_ret = Prepare();
  if (prepare_ret != RET_OK) {
    MS_LOG(ERROR) << "Prepare fail!ret: " << prepare_ret;
    return prepare_ret;
  }

  auto ret = InitTmpBuffer();
  if (ret != RET_OK) {
    MS_LOG(ERROR) << "Init tmp buffer failed.";
    return RET_ERROR;
  }

  int error_code = ParallelLaunch(this->context_->thread_pool_, ConvolutionWinogradImpl, this, thread_count_);
  if (error_code != RET_OK) {
    MS_LOG(ERROR) << "conv winograd error error_code[" << error_code << "]";
    FreeTmpBuffer();
    return RET_ERROR;
  }

  FreeTmpBuffer();
  return RET_OK;
}
}  // namespace mindspore::kernel
