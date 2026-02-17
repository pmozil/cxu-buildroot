#include <iostream>
#include <vector>
#include <stdint.h>
#include <algorithm>
#include <chrono>

#include "cfu.h"

static inline int32_t pack4_i8(int8_t b0, int8_t b1, int8_t b2, int8_t b3) {
  return ((uint32_t)(uint8_t)b0) |
         (((uint32_t)(uint8_t)b1) << 8) |
         (((uint32_t)(uint8_t)b2) << 16) |
         (((uint32_t)(uint8_t)b3) << 24);
}

static inline int32_t hw_simd_dot_product(int32_t packed_a, int32_t packed_b) {
  return cfu_op(1, 0, packed_a, packed_b);
}

int8_t SoftwareReferenceDotProduct(const int8_t* input, const int8_t* weights,
                                   int length, int32_t input_zp, int32_t weight_zp,
                                   int32_t multiplier, int shift, int32_t output_zp) {
  int32_t acc = 0;
  for (int i = 0; i < length; ++i) {
    int32_t in_val = input[i] - input_zp;
    int32_t w_val = weights[i] - weight_zp;
    acc += in_val * w_val;
  }

  acc = (acc * multiplier) >> shift;
  acc += output_zp;
  acc = std::max(acc, (int32_t)-128);
  acc = std::min(acc, (int32_t)127);
  return (int8_t)acc;
}

int main() {
  std::cout << "--- Testing Standalone RISC-V CFU Hardware Layer ---" << std::endl;

  const int input_dim = 16;
  int32_t input_zp = -5, weight_zp = 0, output_zp = -10;
  int32_t multiplier = 1, shift = 4;

  std::vector<int8_t> input_data(input_dim);
  std::vector<int8_t> weight_data(input_dim);
  for (int i = 0; i < input_dim; ++i) {
    input_data[i] = i;
    weight_data[i] = (i % 3) - 1;
  }

  const int ITERATIONS = 100000;
  int8_t sw_result = 0, hw_result = 0;

  auto sw_start = std::chrono::high_resolution_clock::now();

  for (int iter = 0; iter < ITERATIONS; ++iter) {
    sw_result = SoftwareReferenceDotProduct(
        input_data.data(), weight_data.data(), input_dim,
        input_zp, weight_zp, multiplier, shift, output_zp);
  }

  auto sw_end = std::chrono::high_resolution_clock::now();
  double sw_duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(sw_end - sw_start).count();

  auto hw_start = std::chrono::high_resolution_clock::now();

  for (int iter = 0; iter < ITERATIONS; ++iter) {
    int32_t hw_acc = 0;

    for (int in_c = 0; in_c < input_dim; in_c += 4) {
      int8_t a0 = input_data[in_c]     - input_zp;
      int8_t a1 = input_data[in_c + 1] - input_zp;
      int8_t a2 = input_data[in_c + 2] - input_zp;
      int8_t a3 = input_data[in_c + 3] - input_zp;

      int8_t w0 = weight_data[in_c]     - weight_zp;
      int8_t w1 = weight_data[in_c + 1] - weight_zp;
      int8_t w2 = weight_data[in_c + 2] - weight_zp;
      int8_t w3 = weight_data[in_c + 3] - weight_zp;

      int32_t packed_a = pack4_i8(a0, a1, a2, a3);
      int32_t packed_w = pack4_i8(w0, w1, w2, w3);

      hw_acc += hw_simd_dot_product(packed_a, packed_w);
    }

    hw_acc = (hw_acc * multiplier) >> shift;
    hw_acc += output_zp;
    hw_acc = std::max(hw_acc, (int32_t)-128);
    hw_acc = std::min(hw_acc, (int32_t)127);
    hw_result = (int8_t)hw_acc;
  }

  auto hw_end = std::chrono::high_resolution_clock::now();
  double hw_duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(hw_end - hw_start).count();

  std::cout << "Software Time (Average): " << (sw_duration_ns / ITERATIONS) << " ns per run" << std::endl;
  std::cout << "Hardware Time (Average): " << (hw_duration_ns / ITERATIONS) << " ns per run" << std::endl;

  float speedup = sw_duration_ns / hw_duration_ns;
  std::cout << "Speedup:                 " << speedup << "x faster!" << std::endl;

  std::cout << "----------------------------------------------------" << std::endl;
  if (sw_result == hw_result) {
      std::cout << "VALIDATION PASSED: Math matches perfectly." << std::endl;
  } else {
      std::cout << "MISMATCH: Software = " << (int)sw_result << ", Hardware = " << (int)hw_result << std::endl;
  }

  return 0;
}
