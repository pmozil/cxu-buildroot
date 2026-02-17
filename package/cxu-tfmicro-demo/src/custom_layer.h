#ifndef CUSTOM_LAYER_H
#define CUSTOM_LAYER_H

#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"

TFLMRegistration *Register_MY_CUSTOM_LAYER();

template <unsigned int tOpCount>
TfLiteStatus
AddMyCustomLayer(tflite::MicroMutableOpResolver<tOpCount> &resolver) {
  return resolver.AddCustom("MyCustomLayer", Register_MY_CUSTOM_LAYER());
}

#endif
