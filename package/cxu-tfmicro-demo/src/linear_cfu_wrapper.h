#ifndef LINEAR_CFU_WRAPPER_H
#define LINEAR_CFU_WRAPPER_H

#include "cfu.h"
#include <stdint.h>

static inline int32_t pack4_i8(int8_t b0, int8_t b1, int8_t b2, int8_t b3) {
  return ((uint32_t)(uint8_t)b0) | (((uint32_t)(uint8_t)b1) << 8) |
         (((uint32_t)(uint8_t)b2) << 16) | (((uint32_t)(uint8_t)b3) << 24);
}

static inline int32_t hw_simd_dot_product(int32_t packed_a, int32_t packed_b) {
  return cfu_op(1, 0, packed_a, packed_b);
}

#endif // LINEAR_CFU_WRAPPER_H
