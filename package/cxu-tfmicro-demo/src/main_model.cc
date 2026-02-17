#include <iostream>
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_time.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include "custom_layer.h"

extern const unsigned char g_standard_model[];
extern const unsigned char g_custom_model[];

constexpr int kTensorArenaSize = 10 * 1024;
uint8_t tensor_arena[kTensorArenaSize];

int32_t RunAndBenchmark(const unsigned char* model_data,
                        tflite::MicroOpResolver& resolver,
                        int8_t* output_result) {
  const tflite::Model* model = tflite::GetModel(model_data);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    std::cerr << "Model version mismatch" << std::endl;
    return -1;
  }

  tflite::MicroInterpreter interpreter(
      model, resolver, tensor_arena, kTensorArenaSize);

  if (interpreter.AllocateTensors() != kTfLiteOk) {
    std::cerr << "Failed to allocate tensors." << std::endl;
    return -1;
  }

  TfLiteTensor* input = interpreter.input(0);
  for (int i = 0; i < input->bytes; ++i) {
    input->data.int8[i] = i % 127;
  }

  std::cout << "Invoking Interpreter..." << std::endl;
  int32_t start_ticks = tflite::GetCurrentTimeTicks();

  if (interpreter.Invoke() != kTfLiteOk) {
    std::cerr << "Invoke failed" << std::endl;
    return -1;
  }

  int32_t end_ticks = tflite::GetCurrentTimeTicks();
  int32_t duration = end_ticks - start_ticks;

  TfLiteTensor* output = interpreter.output(0);
  *output_result = output->data.int8[0];

  return duration;
}

int main() {
  std::cout << "--- TFLite Micro CFU Accelerator Benchmark ---" << std::endl;

  tflite::MicroMutableOpResolver<2> resolver;

  if (resolver.AddFullyConnected() != kTfLiteOk) {
    std::cerr << "Failed to add standard FullyConnected." << std::endl;
    return -1;
  }

  if (AddMyCustomLayer(resolver) != kTfLiteOk) {
    std::cerr << "Failed to add MyCustomLayer." << std::endl;
    return -1;
  }

  std::cout << "Ops registered successfully. Running benchmarks..." << std::endl;

  int8_t standard_output = 0;
  int32_t standard_ticks = RunAndBenchmark(g_standard_model, resolver, &standard_output);
  std::cout << "[Standard Software] Cycles: " << standard_ticks
            << " | Output[0]: " << (int)standard_output << std::endl;

  int8_t custom_output = 0;
  int32_t custom_ticks = RunAndBenchmark(g_custom_model, resolver, &custom_output);
  std::cout << "[CFU Accelerated] Cycles: " << custom_ticks
            << " | Output[0]: " << (int)custom_output << std::endl;

  std::cout << "----------------------------------------------" << std::endl;
  if (standard_output == custom_output) {
    std::cout << "VALIDATION PASSED: Outputs match exactly." << std::endl;
  } else {
    std::cout << "VALIDATION FAILED: Math mismatch. Check zero-points" << std::endl;
  }

  if (standard_ticks > 0 && custom_ticks > 0) {
    float speedup = (float)standard_ticks / (float)custom_ticks;
    std::cout << "SPEEDUP: " << speedup << "x faster using the CFU" << std::endl;
  }

  std::cout << "----------------------------------------------" << std::endl;

  return 0;
}
