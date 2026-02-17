#include "custom_layer.h"
#include "linear_cfu_wrapper.h"
#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/kernels/internal/tensor_ctypes.h"
#include "tensorflow/lite/kernels/internal/common.h"
#include <iostream>
#include <algorithm>

constexpr int kInputTensor = 0;
constexpr int kWeightsTensor = 1;
constexpr int kBiasTensor = 2;
constexpr int kOutputTensor = 0;

struct CustomLayerData {
  int32_t output_multiplier;
  int output_shift;
};

void* CustomLayerInit(TfLiteContext* context, const char* buffer, size_t length) {
  return context->AllocatePersistentBuffer(context, sizeof(CustomLayerData));
}

void CustomLayerFree(TfLiteContext* context, void* buffer) {
}

TfLiteStatus CustomLayerPrepare(TfLiteContext* context, TfLiteNode* node) {
  TF_LITE_ENSURE_EQ(context, node->inputs->size, 3);
  TF_LITE_ENSURE_EQ(context, node->outputs->size, 1);

  const TfLiteTensor* input = context->GetTensor(context, node->inputs->data[kInputTensor]);
  const TfLiteTensor* weights = context->GetTensor(context, node->inputs->data[kWeightsTensor]);
  TfLiteTensor* output = context->GetTensor(context, node->outputs->data[kOutputTensor]);

  TF_LITE_ENSURE_TYPES_EQ(context, input->type, kTfLiteInt8);
  TF_LITE_ENSURE_TYPES_EQ(context, weights->type, kTfLiteInt8);
  TF_LITE_ENSURE_TYPES_EQ(context, output->type, kTfLiteInt8);

  CustomLayerData* data = static_cast<CustomLayerData*>(node->user_data);

  double real_multiplier = (input->params.scale * weights->params.scale) / output->params.scale;

  tflite::QuantizeMultiplier(real_multiplier, &data->output_multiplier, &data->output_shift);

  return kTfLiteOk;
}

TfLiteStatus CustomLayerEval(TfLiteContext* context, TfLiteNode* node) {
  CustomLayerData* data = static_cast<CustomLayerData*>(node->user_data);

  const TfLiteEvalTensor* input = context->GetEvalTensor(context, node->inputs->data[kInputTensor]);
  const TfLiteEvalTensor* weights = context->GetEvalTensor(context, node->inputs->data[kWeightsTensor]);
  const TfLiteEvalTensor* bias = context->GetEvalTensor(context, node->inputs->data[kBiasTensor]);
  TfLiteEvalTensor* output = context->GetEvalTensor(context, node->outputs->data[kOutputTensor]);

  const int batches = input->dims->data[0];
  const int input_dim = input->dims->data[1];
  const int output_dim = weights->dims->data[0];

  const int8_t* input_data = tflite::micro::GetTensorData<int8_t>(input);
  const int8_t* weights_data = tflite::micro::GetTensorData<int8_t>(weights);
  const int32_t* bias_data = bias ? tflite::micro::GetTensorData<int32_t>(bias) : nullptr;
  int8_t* output_data = tflite::micro::GetTensorData<int8_t>(output);

  const int32_t input_zp = context->GetTensor(context, node->inputs->data[kInputTensor])->params.zero_point;
  const int32_t weight_zp = context->GetTensor(context, node->inputs->data[kWeightsTensor])->params.zero_point;
  const int32_t output_zp = context->GetTensor(context, node->outputs->data[kOutputTensor])->params.zero_point;

  for (int b = 0; b < batches; ++b) {
    for (int out_c = 0; out_c < output_dim; ++out_c) {
        int32_t acc = 0;
        int in_c = 0;
        int limit_simd = input_dim & ~3;

        for (; in_c < limit_simd; in_c += 4) {
          int8_t a0 = input_data[b * input_dim + in_c]     - input_zp;
          int8_t a1 = input_data[b * input_dim + in_c + 1] - input_zp;
          int8_t a2 = input_data[b * input_dim + in_c + 2] - input_zp;
          int8_t a3 = input_data[b * input_dim + in_c + 3] - input_zp;

          int8_t w0 = weights_data[out_c * input_dim + in_c]     - weight_zp;
          int8_t w1 = weights_data[out_c * input_dim + in_c + 1] - weight_zp;
          int8_t w2 = weights_data[out_c * input_dim + in_c + 2] - weight_zp;
          int8_t w3 = weights_data[out_c * input_dim + in_c + 3] - weight_zp;

          int32_t packed_a = pack4_i8(a0, a1, a2, a3);
          int32_t packed_w = pack4_i8(w0, w1, w2, w3);

          acc += hw_simd_dot_product(packed_a, packed_w);
        }

      for (; in_c < input_dim; ++in_c) {
        int32_t input_val = input_data[b * input_dim + in_c] - input_zp;
        int32_t weight_val = weights_data[out_c * input_dim + in_c] - weight_zp;
        acc += input_val * weight_val;
      }

      if (bias_data) {
        acc += bias_data[out_c];
      }

      acc = tflite::MultiplyByQuantizedMultiplier(acc, data->output_multiplier, data->output_shift);

      acc += output_zp;
      acc = std::max(acc, static_cast<int32_t>(-128));
      acc = std::min(acc, static_cast<int32_t>(127));

      output_data[b * output_dim + out_c] = static_cast<int8_t>(acc);
    }
  }

  return kTfLiteOk;
}
