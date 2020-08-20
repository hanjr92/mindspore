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

#include <iostream>
#include <memory>
#include "common/common_test.h"
#include "src/common/file_utils.h"
#include "mindspore/lite/src/kernel_registry.h"
#include "mindspore/lite/src/runtime/kernel/arm/nnacl/pack.h"
#include "mindspore/lite/src/runtime/kernel/arm/nnacl/fp32/matmul.h"
#include "mindspore/lite/src/runtime/kernel/arm/nnacl/int8/deconv.h"
#include "mindspore/lite/src/runtime/kernel/arm/int8/deconvolution_int8.h"

using mindspore::lite::DeviceType;

namespace mindspore {
using mindspore::lite::tensor::QuantArg;
using mindspore::lite::tensor::Tensor;
using mindspore::schema::Format_NHWC;
using mindspore::schema::NodeType_Parameter;
class TestDeconvInt8 : public mindspore::CommonTest {
 public:
  TestDeconvInt8() {}
};

TEST_F(TestDeconvInt8, PackWeight1) {
  int8_t in[] = {-8, 11,   99,  -80, 8,    -12, 37,  -45, 31,   -69, -66, 26,  112, 124, -109, 85,  -24, 28,  -46, 100,
                 72, -36,  -82, 64,  -110, 37,  -72, 65,  -124, 91,  -43, 99,  3,   100, 19,   51,  -14, -81, 67,  90,
                 4,  -106, 105, 28,  -61,  -79, 55,  -54, 47,   -38, 114, 125, -65, 100, 6,    -72, -33, 60,  109, -68};
  int8_t co[] = {-8,   11,  99, -80, 8,   -12, 0,   0,   112, 124,  -109, 85,  -24, 28,  0,   0,   -110, 37,  -72, 65,
                 -124, 91,  0,  0,   -14, -81, 67,  90,  4,   -106, 0,    0,   47,  -38, 114, 125, -65,  100, 0,   0,
                 37,   -45, 31, -69, -66, 26,  0,   0,   -46, 100,  72,   -36, -82, 64,  0,   0,   -43,  99,  3,   100,
                 19,   51,  0,  0,   105, 28,  -61, -79, 55,  -54,  0,    0,   6,   -72, -33, 60,  109,  -68, 0,   0};
  int8_t dst[80] = {0};
  /*5*1*2*6 nhwc*/
  PackNHWCToC8HWN8Int8(in, dst, 5, 2, 6);
  CompareOutputData(dst, co, 80, 1);
}

TEST_F(TestDeconvInt8, PackWeight2) {
  int8_t in[] = {
    40,   24,   94,   122, 67,  34,  -89, 31,   -43, 121,  48,   -54,  44,   -91,  35,   89,   -37, 114,  -8,   103,
    -22,  32,   26,   112, -92, -23, 43,  9,    81,  118,  -73,  -54,  65,   -99,  51,   -90,  121, -62,  119,  -93,
    21,   -92,  -1,   -82, -71, -54, 63,  -93,  92,  -93,  99,   122,  -104, -16,  -8,   -32,  90,  -126, 51,   91,
    4,    70,   -7,   116, 99,  81,  -79, 124,  -14, 28,   97,   9,    -97,  99,   88,   -15,  54,  26,   77,   -25,
    113,  119,  119,  -75, -17, 7,   7,   1,    69,  66,   40,   -13,  80,   -115, -98,  -8,   -17, 31,   88,   65,
    -1,   -15,  -98,  77,  56,  119, -20, -32,  -54, -58,  -16,  52,   121,  126,  -33,  43,   92,  -34,  -17,  -52,
    104,  -52,  -91,  76,  79,  105, 102, -65,  43,  32,   13,   15,   -38,  95,   -18,  -82,  -7,  118,  -79,  -85,
    120,  -15,  2,    32,  -94, 111, 115, 102,  -18, 121,  -106, 54,   63,   111,  -16,  92,   82,  -23,  111,  53,
    1,    -48,  45,   19,  -4,  -15, -72, 41,   80,  -51,  116,  31,   94,   101,  -10,  18,   0,   -49,  108,  28,
    -36,  47,   -14,  -2,  -10, 31,  -92, -84,  74,  -114, -107, 66,   99,   -121, -107, 31,   -38, 56,   -30,  109,
    -7,   28,   -22,  -17, -3,  -2,  27,  -3,   108, -84,  -23,  -71,  -54,  20,   -45,  109,  -42, 78,   -79,  98,
    -10,  57,   52,   1,   25,  73,  21,  -78,  46,  121,  66,   92,   24,   55,   4,    -110, -37, 112,  -18,  10,
    -42,  16,   -9,   31,  39,  -70, 108, -3,   -90, -60,  -121, 11,   50,   -88,  -104, -29,  -89, 94,   64,   -91,
    -101, -7,   23,   -57, 93,  16,  17,  35,   -48, -25,  13,   -121, 73,   -68,  -54,  -122, -20, 12,   64,   20,
    -11,  -6,   -71,  -52, -97, 109, 116, -107, 117, -124, 56,   80,   -108, 30,   123,  56,   -80, 39,   -18,  -97,
    -103, 122,  114,  -10, -31, 97,  -92, 105,  -61, -25,  10,   -119, -106, 41,   77,   -117, 55,  -83,  -29,  14,
    27,   -106, -86,  41,  43,  23,  11,  -76,  -34, 121,  94,   18,   69,   73,   100,  54,   43,  32,   13,   15,
    -38,  95,   -18,  -82, -7,  118, -79, -85,  120, -15,  2,    32,   -94,  111,  115,  102,  -18, 121,  -106, 54,
    63,   111,  -16,  92,  82,  -23, 111, 53,   1,   -48,  45,   19,   -4,   -15,  -72,  41,   80,  -51,  116,  31,
    94,   101,  -10,  18,  0,   -49, 108, 28,   -36, 47,   -14,  -2,   -10,  31,   -92,  -84,  74,  -114, -107, 66,
    99,   -121, -107, 31,  -38, 56,  -30, 109,  -7,  28,   -22,  -17,  -3,   -2,   27,   -3,   108, -84,  -23,  -71,
    -54,  20,   -45,  109, -42, 78,  -79, 98,   -10, 57,   52,   1,    25,   73,   21,   -78,  46,  121,  66,   92};
  int8_t co[] = {
    40,   24,   94,   122,  67,   34,   -89,  31,   -22, 32,   26,   112,  -92,  -23,  43,   9,   21,   -92, -1,   -82,
    -71,  -54,  63,   -93,  4,    70,   -7,   116,  99,  81,   -79,  124,  113,  119,  119,  -75, -17,  7,   7,    1,
    -1,   -15,  -98,  77,   56,   119,  -20,  -32,  104, -52,  -91,  76,   79,   105,  102,  -65, 120,  -15, 2,    32,
    -94,  111,  115,  102,  1,    -48,  45,   19,   -4,  -15,  -72,  41,   -36,  47,   -14,  -2,  -10,  31,  -92,  -84,
    -7,   28,   -22,  -17,  -3,   -2,   27,   -3,   -10, 57,   52,   1,    25,   73,   21,   -78, -42,  16,  -9,   31,
    39,   -70,  108,  -3,   -101, -7,   23,   -57,  93,  16,   17,   35,   -11,  -6,   -71,  -52, -97,  109, 116,  -107,
    -103, 122,  114,  -10,  -31,  97,   -92,  105,  27,  -106, -86,  41,   43,   23,   11,   -76, -38,  95,  -18,  -82,
    -7,   118,  -79,  -85,  63,   111,  -16,  92,   82,  -23,  111,  53,   94,   101,  -10,  18,  0,    -49, 108,  28,
    99,   -121, -107, 31,   -38,  56,   -30,  109,  -54, 20,   -45,  109,  -42,  78,   -79,  98,  -43,  121, 48,   -54,
    44,   -91,  35,   89,   81,   118,  -73,  -54,  65,  -99,  51,   -90,  92,   -93,  99,   122, -104, -16, -8,   -32,
    -14,  28,   97,   9,    -97,  99,   88,   -15,  69,  66,   40,   -13,  80,   -115, -98,  -8,  -54,  -58, -16,  52,
    121,  126,  -33,  43,   43,   32,   13,   15,   -38, 95,   -18,  -82,  -18,  121,  -106, 54,  63,   111, -16,  92,
    80,   -51,  116,  31,   94,   101,  -10,  18,   74,  -114, -107, 66,   99,   -121, -107, 31,  108,  -84, -23,  -71,
    -54,  20,   -45,  109,  46,   121,  66,   92,   24,  55,   4,    -110, -90,  -60,  -121, 11,  50,   -88, -104, -29,
    -48,  -25,  13,   -121, 73,   -68,  -54,  -122, 117, -124, 56,   80,   -108, 30,   123,  56,  -61,  -25, 10,   -119,
    -106, 41,   77,   -117, -34,  121,  94,   18,   69,  73,   100,  54,   120,  -15,  2,    32,  -94,  111, 115,  102,
    1,    -48,  45,   19,   -4,   -15,  -72,  41,   -36, 47,   -14,  -2,   -10,  31,   -92,  -84, -7,   28,  -22,  -17,
    -3,   -2,   27,   -3,   -10,  57,   52,   1,    25,  73,   21,   -78,  -37,  114,  -8,   103, 0,    0,   0,    0,
    121,  -62,  119,  -93,  0,    0,    0,    0,    90,  -126, 51,   91,   0,    0,    0,    0,   54,   26,  77,   -25,
    0,    0,    0,    0,    -17,  31,   88,   65,   0,   0,    0,    0,    92,   -34,  -17,  -52, 0,    0,   0,    0,
    -7,   118,  -79,  -85,  0,    0,    0,    0,    82,  -23,  111,  53,   0,    0,    0,    0,   0,    -49, 108,  28,
    0,    0,    0,    0,    -38,  56,   -30,  109,  0,   0,    0,    0,    -42,  78,   -79,  98,  0,    0,   0,    0,
    -37,  112,  -18,  10,   0,    0,    0,    0,    -89, 94,   64,   -91,  0,    0,    0,    0,   -20,  12,  64,   20,
    0,    0,    0,    0,    -80,  39,   -18,  -97,  0,   0,    0,    0,    55,   -83,  -29,  14,  0,    0,   0,    0,
    43,   32,   13,   15,   0,    0,    0,    0,    -18, 121,  -106, 54,   0,    0,    0,    0,   80,   -51, 116,  31,
    0,    0,    0,    0,    74,   -114, -107, 66,   0,   0,    0,    0,    108,  -84,  -23,  -71, 0,    0,   0,    0,
    46,   121,  66,   92,   0,    0,    0,    0};
  int8_t dst[528] = {0};
  PackNHWCToC8HWN8Int8(in, dst, 22, 1, 20);
  CompareOutputData(dst, co, 528, 1);
}

TEST_F(TestDeconvInt8, MatMulTest1) {
  int8_t a_row_major_10_12[] = {
    -6, 76,  32,  80,  -73, 8,   -85, -3,  114, 80,  30,  42,  -41, 117,  62,  -76, -77, -111, 88,  105,
    68, 105, -74, 13,  51,  94,  31,  -52, -92, -4,  -35, -71, 101, -93,  46,  -65, 57,  -41,  -51, 77,
    1,  9,   73,  -19, -36, 57,  81,  -24, 40,  103, 112, 109, -41, -68,  57,  61,  55,  -20,  3,   2,
    17, -16, -31, 58,  -4,  67,  -4,  -95, -5,  -72, 81,  15,  -7,  -16,  -47, 112, 114, -26,  -98, 53,
    15, -49, 26,  19,  19,  8,   -57, -35, -79, 118, 29,  21,  37,  -48,  83,  7,   124, 113,  -5,  15,
    -8, 107, -65, -88, 50,  -47, -80, -84, 3,   -45, 92,  42,  -20, -101, 106, -10, 89,  67,   55,  10};
  int32_t zp_a = 15;
  int8_t a_col8_major[16 * 12] = {0};
  int8_t b_col_major_12_18[] = {
    92,  27,   22,   52,  -112, -20, -57, -2,   89,   32,  93,   -66,  -25, -54, 94,  -97, -119, -98,  101,  -99,
    77,  -83,  76,   95,  59,   97,  8,   40,   -109, -20, 67,   -107, 37,  -6,  -54, -20, -30,  36,   -106, -103,
    -3,  -86,  -82,  59,  4,    -75, -50, -106, 55,   104, -117, -71,  -20, -85, -77, 16,  -25,  -58,  4,    80,
    -75, 94,   32,   -68, 2,    40,  56,  -103, 11,   -98, -70,  -69,  0,   57,  -6,  82,  66,   -112, -61,  33,
    -77, -53,  95,   -38, 87,   -46, -3,  81,   -47,  43,  21,   26,   -45, -57, 50,  -24, -82,  -114, 61,   46,
    -53, 78,   -24,  31,  -7,   37,  29,  38,   45,   106, 52,   -42,  31,  -6,  -61, -87, 2,    79,   -5,   -42,
    43,  -106, -104, 7,   91,   -63, 58,  97,   -15,  74,  -96,  15,   -23, -3,  -47, -97, 100,  -54,  26,   -46,
    35,  26,   100,  -80, 34,   -25, 96,  -67,  -80,  -27, 66,   41,   41,  -43, -43, -38, -4,   -64,  31,   7,
    -8,  6,    -2,   39,  -119, 53,  75,  -91,  -44,  77,  -62,  22,   -44, 78,  -67, -48, -115, -4,   43,   81,
    40,  -20,  -5,   -89, 60,   -62, -4,  -48,  66,   -64, -69,  62,   17,  -89, 1,   87,  81,   32,   -29,  51,
    40,  27,   66,   67,  11,   -69, 85,  -79,  -106, 55,  22,   -23,  62,  69,  -74, 49};
  int32_t zp_b = -20;
  int8_t b_row8_major[12 * 24] = {0};
  int32_t co_row_major_10_18[] = {
    32005,  3597,   16595,  -3458,  6627,   -6663,  818,    -3910,  10228,  15079,  -19205, -10203, -3178,  -10046,
    10374,  -6199,  5330,   12163,  1819,   20533,  17382,  18283,  9778,   9185,   -12623, -26234, -11987, 7904,
    8144,   -1603,  27611,  -10190, -20053, 4999,   -28389, 21852,  24680,  25858,  23506,  17944,  11768,  24378,
    -6102,  -4675,  -23460, 10434,  -47579, 1986,   12018,  -19418, -7248,  4938,   -32613, -941,   8171,   -4788,
    3325,   -11310, -8351,  -14786, 6909,   16401,  2017,   -6456,  11242,  7393,   -9119,  17312,  2646,   -14402,
    7201,   -9949,  23986,  17607,  27461,  -1547,  2783,   7558,   19487,  11158,  -2686,  6328,   -8225,  -11668,
    21858,  -2079,  -8671,  -639,   -1544,  1235,   1156,   6582,   2829,   -10311, -2692,  5154,   1527,   10870,
    106,    -8189,  -24174, -1846,  -15399, -3598,  14874,  -5591,  -619,   -13667, -6053,  -31103, -24499, 13008,
    9143,   -17982, 28437,  2176,   -2114,  -11631, 10779,  -1032,  -24690, -3112,  2125,   432,    20270,  -33859,
    8907,   10063,  1603,   3761,   4805,   4904,   -15594, 10786,  4287,   -13591, -18777, -1679,  2109,   -2243,
    12051,  -8504,  -6558,  4209,   13606,  -25803, 27922,  12092,  7140,   27142,  -12267, 2339,   -26224, 23674,
    -26579, -11398, -1823,  -18976, 3641,   4415,   -24878, -2045,  15937,  41465,  12601,  -14513, -17619, -5728,
    334,    -424,   8147,   -1369,  5984,   11000,  19016,  4456,   -25920, 4506,   5930,   15458};
  int32_t c_row8x8_major[16 * 24] = {0};

  int32_t out_row_major[180] = {0};
  RowMajor2Col8MajorInt8(a_row_major_10_12, a_col8_major, 10, 12);
  RowMajor2Col8MajorInt8(b_col_major_12_18, b_row8_major, 18, 12);
  MatMulInt8(a_col8_major, b_row8_major, c_row8x8_major, 16, 24, 12, zp_a, zp_b);
  Row8x8Major2RowMajor(reinterpret_cast<float *>(c_row8x8_major), reinterpret_cast<float *>(out_row_major), 10, 18, 18);
  CompareOutputData(out_row_major, co_row_major_10_18, 180, 1);
}

TEST_F(TestDeconvInt8, MatMulOptTest1) {
  int8_t a_src_ptr[] = {-6, 76,  32,  80,  -73, 8,    -85, -3,  114, 80,  30,  42,  -41, 117, 62,  -76, -77, -111,
                        88, 105, 68,  105, -74, 13,   51,  94,  31,  -52, -92, -4,  -35, -71, 101, -93, 46,  -65,
                        57, -41, -51, 77,  1,   9,    73,  -19, -36, 57,  81,  -24, 40,  103, 112, 109, -41, -68,
                        57, 61,  55,  -20, 3,   2,    17,  -16, -31, 58,  -4,  67,  -4,  -95, -5,  -72, 81,  15,
                        -7, -16, -47, 112, 114, -26,  -98, 53,  15,  -49, 26,  19,  19,  8,   -57, -35, -79, 118,
                        29, 21,  37,  -48, 83,  7,    124, 113, -5,  15,  -8,  107, -65, -88, 50,  -47, -80, -84,
                        3,  -45, 92,  42,  -20, -101, 106, -10, 89,  67,  55,  10};
  int32_t input_zp = 15;
  int8_t b_src_ptr[] = {
    92,  27,   22,   52,  -112, -20, -57, -2,   89,   32,  93,   -66,  -25, -54, 94,  -97, -119, -98,  101,  -99,
    77,  -83,  76,   95,  59,   97,  8,   40,   -109, -20, 67,   -107, 37,  -6,  -54, -20, -30,  36,   -106, -103,
    -3,  -86,  -82,  59,  4,    -75, -50, -106, 55,   104, -117, -71,  -20, -85, -77, 16,  -25,  -58,  4,    80,
    -75, 94,   32,   -68, 2,    40,  56,  -103, 11,   -98, -70,  -69,  0,   57,  -6,  82,  66,   -112, -61,  33,
    -77, -53,  95,   -38, 87,   -46, -3,  81,   -47,  43,  21,   26,   -45, -57, 50,  -24, -82,  -114, 61,   46,
    -53, 78,   -24,  31,  -7,   37,  29,  38,   45,   106, 52,   -42,  31,  -6,  -61, -87, 2,    79,   -5,   -42,
    43,  -106, -104, 7,   91,   -63, 58,  97,   -15,  74,  -96,  15,   -23, -3,  -47, -97, 100,  -54,  26,   -46,
    35,  26,   100,  -80, 34,   -25, 96,  -67,  -80,  -27, 66,   41,   41,  -43, -43, -38, -4,   -64,  31,   7,
    -8,  6,    -2,   39,  -119, 53,  75,  -91,  -44,  77,  -62,  22,   -44, 78,  -67, -48, -115, -4,   43,   81,
    40,  -20,  -5,   -89, 60,   -62, -4,  -48,  66,   -64, -69,  62,   17,  -89, 1,   87,  81,   32,   -29,  51,
    40,  27,   66,   67,  11,   -69, 85,  -79,  -106, 55,  22,   -23,  62,  69,  -74, 49};
  int32_t filter_zp = -20;

  /*
   * ----------------------   pack input  ------------------------- */
  int8_t packed_a[12 * 16] = {0};
  memset(packed_a, static_cast<int8_t>(input_zp), 12 * 16);
  int8_t correct_packed_a[] = {
    -6,  76,  32,  80,  -73, 8,   -85, -3,  114, 80,  30,  42,  15,  15,  15,  15,  -41, 117,  62,  -76, -77, -111,
    88,  105, 68,  105, -74, 13,  15,  15,  15,  15,  51,  94,  31,  -52, -92, -4,  -35, -71,  101, -93, 46,  -65,
    15,  15,  15,  15,  57,  -41, -51, 77,  1,   9,   73,  -19, -36, 57,  81,  -24, 15,  15,   15,  15,  40,  103,
    112, 109, -41, -68, 57,  61,  55,  -20, 3,   2,   15,  15,  15,  15,  17,  -16, -31, 58,   -4,  67,  -4,  -95,
    -5,  -72, 81,  15,  15,  15,  15,  15,  -7,  -16, -47, 112, 114, -26, -98, 53,  15,  -49,  26,  19,  15,  15,
    15,  15,  19,  8,   -57, -35, -79, 118, 29,  21,  37,  -48, 83,  7,   15,  15,  15,  15,   124, 113, -5,  15,
    -8,  107, -65, -88, 50,  -47, -80, -84, 15,  15,  15,  15,  3,   -45, 92,  42,  -20, -101, 106, -10, 89,  67,
    55,  10,  15,  15,  15,  15,  15,  15,  15,  15,  15,  15,  15,  15,  15,  15,  15,  15,   15,  15,  15,  15,
    15,  15,  15,  15,  15,  15,  15,  15,  15,  15,  15,  15,  15,  15,  15,  15,
  };
  RowMajor2Row16x4MajorInt8(a_src_ptr, packed_a, 10, 12);
  CompareOutputData(packed_a, correct_packed_a, 16 * 12, 0);

  /*
   * ----------------------   pack weight  ------------------------- */
  int8_t packed_b[16 * 3 * 8] = {0};
  memset(packed_b, static_cast<int8_t>(filter_zp), 16 * 3 * 8);
  int8_t correct_packed_b[] = {
    92,   101,  -30,  -77,  0,    21,   45,  58,  34,   -2,  40,  -29, -20, -20,  -20,  -20, 27,  -99,  36,  16,  57,
    26,   106,  97,   -25,  39,   -20,  51,  -20, -20,  -20, -20, 22,  77,  -106, -25,  -6,  -45, 52,   -15, 96,  -119,
    -5,   40,   -20,  -20,  -20,  -20,  52,  -83, -103, -58, 82,  -57, -42, 74,   -67,  53,  -89, 27,   -20, -20, -20,
    -20,  -112, 76,   -3,   4,    66,   50,  31,  -96,  -80, 75,  60,  66,  -20,  -20,  -20, -20, -20,  95,  -86, 80,
    -112, -24,  -6,   15,   -27,  -91,  -62, 67,  -20,  -20, -20, -20, -20, -20,  -20,  -20, -20, -20,  -20, -20, -20,
    -20,  -20,  -20,  -20,  -20,  -20,  -20, -20, -20,  -20, -20, -20, -20, -20,  -20,  -20, -20, -20,  -20, -20, -20,
    -20,  -20,  -57,  59,   -82,  -75,  -61, -82, -61,  -23, 66,  -44, -4,  11,   -20,  -20, -20, -20,  -2,  97,  59,
    94,   33,   -114, -87,  -3,   41,   77,  -48, -69,  -20, -20, -20, -20, 89,   8,    4,   32,  -77,  61,  2,   -47,
    41,   -62,  66,   85,   -20,  -20,  -20, -20, 32,   40,  -75, -68, -53, 46,   79,   -97, -43, 22,   -64, -79, -20,
    -20,  -20,  -20,  93,   -109, -50,  2,   95,  -53,  -5,  100, -43, -44, -69,  -106, -20, -20, -20,  -20, -66, -20,
    -106, 40,   -38,  78,   -42,  -54,  -38, 78,  62,   55,  -20, -20, -20, -20,  -20,  -20, -20, -20,  -20, -20, -20,
    -20,  -20,  -20,  -20,  -20,  -20,  -20, -20, -20,  -20, -20, -20, -20, -20,  -20,  -20, -20, -20,  -20, -20, -20,
    -20,  -20,  -20,  -20,  -25,  67,   55,  56,  87,   -24, 43,  26,  -4,  -67,  17,   22,  -20, -20,  -20, -20, -54,
    -107, 104,  -103, -46,  31,   -106, -46, -64, -48,  -89, -23, -20, -20, -20,  -20,  94,  37,  -117, 11,  -3,  -7,
    -104, 35,   31,   -115, 1,    62,   -20, -20, -20,  -20, -97, -6,  -71, -98,  81,   37,  7,   26,   7,   -4,  87,
    69,   -20,  -20,  -20,  -20,  -119, -54, -20, -70,  -47, 29,  91,  100, -8,   43,   81,  -74, -20,  -20, -20, -20,
    -98,  -20,  -85,  -69,  43,   38,   -63, -80, 6,    81,  32,  49,  -20, -20,  -20,  -20, -20, -20,  -20, -20, -20,
    -20,  -20,  -20,  -20,  -20,  -20,  -20, -20, -20,  -20, -20, -20, -20, -20,  -20,  -20, -20, -20,  -20, -20, -20,
    -20,  -20,  -20,  -20,  -20,  -20};
  DeConvWeightTransInt8(b_src_ptr, packed_b, 12, 6, 3, true);
  /* kernel : 12x1x3x6   nhwc   */
  CompareOutputData(packed_b, correct_packed_b, 16 * 3 * 8, 0);

  /*
   * ----------------------   calculate input_sum   ------------------------- */
  int32_t input_sum[12] = {0};
  int32_t correct_input_sum[] = {-7100, -4780, 580, -4880, -9460, -1420, -3120, -3260, -1840, -6960, -4800, -4800};
  DeConvPackInputSum(packed_a, input_sum, filter_zp, 12, 16, true);
  CompareOutputData(input_sum, correct_input_sum, 12, 0);

  for (int i = 0; i < 12; i++) {
    if (input_sum[i] != correct_input_sum[i]) {
      printf("%d %d %d\n", i, input_sum[i], correct_input_sum[i]);
    }
  }

  /*
   * ----------------------   calculate weight_sum   ------------------------- */
  int32_t weight_sum[3 * 8] = {0};
  int32_t correct_weight_sum[] = {-7395, -8265, -3090, -435, -5655, -1035, 0,     0,     1695,  -4770, -6630, 300,
                                  -765,  -2835, 0,     0,    -7395, 4665,  -2475, -4170, -2880, -1110, 0,     0};
  DeConvPackWeightSum(packed_b, weight_sum, input_zp, filter_zp, 16, 24, true);
  CompareOutputData(weight_sum, correct_weight_sum, 3 * 8, 0);

  /*
   * ----------------------   do matmul   ------------------------- */
  int32_t tmp_output[12 * 24] = {0};
  int32_t correct_tmp_output[] = {
    -1624,  -19061, 1795,   -17119, 14706,  417,    7306,   1357,   9653,   -44022, 19414,  -36187, -2041,  6874,
    -5766,  3072,   9842,   2395,   12464,  -18826, -12267, -17853, 4617,   -19468, -15734, -6112,  2122,   14259,
    11098,  -9520,  12407,  -15239, 10309,  -34271, 9740,   -14607, -5027,  12313,  -508,   -10808, 0,      0,
    0,      0,      0,      0,      0,      0,      1604,   14898,  0,      0,      -8212,  9471,   0,      0,
    -23430, 6343,   0,      0,      4020,   -3740,  0,      0,      -9730,  22378,  0,      0,      4702,   4740,
    0,      0,      -7541,  5461,   0,      0,      -6633,  8356,   0,      0,      -16854, 9147,   0,      0,
    -4018,  -11524, 0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      17194,  28501,
    13376,  -9359,  21454,  22425,  -21049, 6603,   23479,  -658,   12866,  9739,   -12173, -7558,  3862,   10238,
    4110,   31945,  10069,  -7376,  -1948,  -20322, 16439,  3260,   1712,   12743,  -8132,  -27744, 7633,   -33916,
    18755,  11300,  3686,   9222,   10103,  26102,  17,     13135,  785,    -6305,  0,      0,      0,      0,
    0,      0,      0,      0,      -27325, 14957,  0,      0,      -12191, -21866, 0,      0,      -21690, -18554,
    0,      0,      8737,   14529,  0,      0,      -1774,  -19575, 0,      0,      -12761, 13286,  0,      0,
    20523,  2488,   0,      0,      -12782, 12688,  0,      0,      -1194,  -10523, 0,      0,      -4044,  -9671,
    0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      -4671,  -4173,  8675,   -8560,
    -1597,  -4946,  -20214, -6752,  -11439, 5138,   11119,  -17661, -6690,  -17301, -5541,  -4356,  22347,  -11778,
    2389,   -22030, -5176,  -242,   8786,   -994,   9104,   -7208,  24117,  3724,   -13648, -1840,  12265,  10347,
    -10325, 7184,   19374,  -29001, 3979,   -6704,  -23278, -8124,  0,      0,      0,      0,      0,      0,
    0,      0,      -9132,  8560,   0,      0,      19264,  -10169, 0,      0,      -15133, -13678, 0,      0,
    7894,   -51,    0,      0,      -4775,  -29785, 0,      0,      -12597, 4088,   0,      0,      -17420, 1815,
    0,      0,      15796,  3101,   0,      0,      -37969, -10818, 0,      0,      12714,  -7827,  0,      0,
    0,      0,      0,      0,      0,      0,      0,      0};
  MatMulOptR4Int8(tmp_output, packed_a, packed_b, weight_sum, input_sum, 12, 24, 16);
  CompareOutputData(tmp_output, correct_tmp_output, 12 * 3 * 8, 0);
}

TEST_F(TestDeconvInt8, PostAddTest1) {
  int32_t in[] = {
    -4956,  -3923,  868,   -8880, -4089, -5179, -4526, -4527, -10464, 99,    -5826, -2995, -4519, -4519, -10509, -2505,
    -11272, 434,    -4522, -4523, -5287, -8936, -878,  373,   -4528,  -4529, -1960, -6589, 1688,  2287,  -8059,  926,
    -2506,  -6972,  -2834, -8281, -8118, -3110, -4526, -4527, -4528,  -4529, -4519, -4519, -4519, -4519, -4519,  -4519,
    -4520,  -4521,  -4522, -4523, -4524, -4525, -4526, -4527, -4528,  -4529, -4519, -4519, -4519, -4519, -4519,  -4519,
    1578,   2231,   -4522, -4523, -4524, -4525, -4526, -4527, -8449,  -990,  -4519, -4519, -4519, -4519, -4519,  -4519,
    -4303,  -10293, -4522, -4523, -4524, -4525, -4526, -4527, -4528,  -4529, -4519, -4519, -4519, -4519, -4519,  -4519,
    -7025,  924,    -4522, -4523, -4524, -4525, -4526, -4527, -4528,  -4529, -4519, -4519, -4519, -4519, -4519,  -4519,
    -4520,  -4521,  -4522, -4523, -4524, -4525, -4526, -4527, -4528,  -4529, -4519, -4519, -4519, -4519, -4519,  -4519};
  int8_t co[] = {-8,  11,  99,  -80,  8,  -12, 0,  0,   112, 124, -109, 85, -24,  28, 0,   0,  -110,
                 37,  -72, 65,  -124, 91, 0,   0,  -14, -81, 67,  90,   4,  -106, 0,  0,   47, -38,
                 114, 125, -65, 100,  0,  0,   37, -45, 31,  -69, -66,  26, 0,    0,  -46, 100};
  int32_t bias[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  int8_t out[50] = {0};
  double multiplier = 0.0183649725490196;
  int32_t quant_multiplier;
  int32_t left_shift;
  int32_t right_shift;
  QuantizeRoundParameter(multiplier, &quant_multiplier, &left_shift, &right_shift);
  int32_t zp = 83;
  PostFuncInt8C8(in, bias, out, 10, 5, quant_multiplier, left_shift, right_shift, zp, -128, 127);
  CompareOutputData(out, co, 50, 1);

  int8_t co_relu[] = {0, 11, 99, 0, 8, 0, 0, 0,  112, 124, 0,   85, 0,   28, 0, 0,  0, 37, 0, 65, 0,  91, 0, 0, 0,
                      0, 67, 90, 4, 0, 0, 0, 47, 0,   114, 125, 0,  100, 0,  0, 37, 0, 31, 0, 0,  26, 0,  0, 0, 100};
  PostFuncInt8C8(in, bias, out, 10, 5, quant_multiplier, left_shift, right_shift, zp, 0, 127);
  CompareOutputData(out, co_relu, 50, 1);

  int8_t co_relu6[] = {0, 6, 6, 0, 6, 0, 0, 0, 6, 6, 0, 6, 0, 6, 0, 0, 0, 6, 0, 6, 0, 6, 0, 0, 0,
                       0, 6, 6, 4, 0, 0, 0, 6, 0, 6, 6, 0, 6, 0, 0, 6, 0, 6, 0, 0, 6, 0, 0, 0, 6};
  PostFuncInt8C8(in, bias, out, 10, 5, quant_multiplier, left_shift, right_shift, zp, 0, 6);
  CompareOutputData(out, co_relu6, 50, 1);
}

int DeConvInt8TestInit1(std::vector<lite::tensor::Tensor *> *inputs_, std::vector<lite::tensor::Tensor *> *outputs_,
                        ConvParameter *conv_param, int8_t **correct) {
  /* float data from deconv fp32 testcase : DeConvTestInit2 */
  /*   vq = (vi - zp) * s     vi = vq / s + zp */
  Tensor *in_t = new Tensor(kNumberTypeInt8, {1, 4, 2, 3}, Format_NHWC, NodeType_Parameter);
  in_t->MallocData();
  int8_t in[] = {6, 43, 38, 24, -8, 12, 41, -24, -20, 41, -19, -6, -26, -6, 23, -31, 34, 45, 8, 45, -39, -27, -48, 12};
  memcpy(in_t->Data(), in, sizeof(int8_t) * in_t->ElementsNum());
  QuantArg *in_quant_arg = new QuantArg();
  in_quant_arg->zeroPoint = -19, in_quant_arg->scale = 0.31228156;
  in_t->AddQuantParam(*in_quant_arg);
  inputs_->push_back(in_t);

  Tensor *weight_t = new Tensor(kNumberTypeInt8, {3, 3, 3, 2}, Format_NHWC, NodeType_Parameter);
  weight_t->MallocData();
  int8_t weight[] = {66, 89, 98, 74,  95, 86, 125, 95, 105, 83, 116, 94, 90, 80, 86, 59, 72, 92,
                     64, 76, 92, 80,  90, 87, 106, 55, 105, 60, 75,  53, 81, 81, 98, 81, 86, 59,
                     74, 82, 97, 105, 71, 67, 79,  87, 72,  79, 80,  76, 96, 80, 83, 71, 61, 79};
  memcpy(weight_t->Data(), weight, sizeof(int8_t) * weight_t->ElementsNum());
  QuantArg *w_quant_arg = new QuantArg();
  w_quant_arg->zeroPoint = 83, w_quant_arg->scale = 0.023649725490196;
  weight_t->AddQuantParam(*w_quant_arg);
  inputs_->push_back(weight_t);

  Tensor *out_t = new Tensor(kNumberTypeInt8, {1, 7, 3, 2}, Format_NHWC, NodeType_Parameter);
  out_t->MallocData();
  QuantArg *out_quant_arg = new QuantArg();
  out_quant_arg->zeroPoint = 31, out_quant_arg->scale = 0.3439215686275;
  out_t->AddQuantParam(*out_quant_arg);
  outputs_->push_back(out_t);

  *correct = reinterpret_cast<int8_t *>(malloc(out_t->ElementsNum() * sizeof(int8_t)));
  int8_t co_nchw[] = {57, 76, 49, 71,  8, 61, 57, 127, 56, 46, -11, 61, 23, 31,  34, 50, 59, 49, 78, 17, 6,
                      -3, -5, 23, -11, 6, -5, 33, 64,  30, 21, 18,  25, 21, -15, 0,  4,  31, 36, 2,  17, 43};
  PackNCHWToNHWCInt8(co_nchw, *correct, out_t->Batch(), out_t->Width() * out_t->Height(), out_t->Channel());

  conv_param->kernel_h_ = conv_param->kernel_w_ = 3;
  conv_param->pad_h_ = conv_param->pad_w_ = 1;
  conv_param->stride_h_ = conv_param->stride_w_ = 2;
  conv_param->dilation_h_ = conv_param->dilation_w_ = 1;
  return out_t->ElementsNum();
}

TEST_F(TestDeconvInt8, DeConvInt8Test1) {
  std::vector<lite::tensor::Tensor *> inputs_;
  std::vector<lite::tensor::Tensor *> outputs_;
  auto deconv_param = new ConvParameter();
  lite::Context *ctx = new lite::Context;
  ctx->thread_num_ = 1;
  int8_t *correct;
  int total_size = DeConvInt8TestInit1(&inputs_, &outputs_, deconv_param, &correct);
  mindspore::kernel::DeConvInt8CPUKernel *deconv = new mindspore::kernel::DeConvInt8CPUKernel(
    reinterpret_cast<OpParameter *>(deconv_param), inputs_, outputs_, ctx, nullptr);

  deconv->Init();
  deconv->Run();
  CompareOutputData(reinterpret_cast<int8_t *>(outputs_[0]->Data()), correct, total_size, 3);

  delete deconv_param;
  delete deconv;
  for (auto t : inputs_) delete t;
  for (auto t : outputs_) delete t;
  free(correct);
}
}  // namespace mindspore