cmake_minimum_required(VERSION 3.16)
project(cxu_tflite_demo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(cxu_tflite_demo
    main.cc
    custom_layer.cc
)

set(TFLM_DOWNLOADS ${CMAKE_SYSROOT}/usr/include/tensorflow/lite/micro/tools/make/downloads)

target_include_directories(cxu_tflite_demo PRIVATE
    ${CMAKE_SYSROOT}/usr/include
    ${TFLM_DOWNLOADS}/flatbuffers/include
    ${TFLM_DOWNLOADS}/gemmlowp
    ${TFLM_DOWNLOADS}/ruy
    ${TFLM_DOWNLOADS}/kissfft
)

target_link_libraries(cxu_tflite_demo PRIVATE
    tensorflow-microlite
    m
)

install(TARGETS cxu_tflite_demo DESTINATION bin)
