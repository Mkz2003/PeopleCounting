/**
  ******************************************************************************
  * @file    network.h
  * @date    2026-02-14T16:27:23+0000
  * @brief   ST.AI Tool Automatic Code Generator for Embedded NN computing
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  ******************************************************************************
  */
#ifndef STAI_NETWORK_DETAILS_H
#define STAI_NETWORK_DETAILS_H

#include "stai.h"
#include "layers.h"

const stai_network_details g_network_details = {
  .tensors = (const stai_tensor[24]) {
   { .size_bytes = 196608, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_U8, .shape = {4, (const int32_t[4]){1, 256, 256, 3}}, .scale = {1, (const float[1]){0.003921568859368563}}, .zeropoint = {1, (const int16_t[1]){0}}, .name = "serving_default_keras_tensor0_output" },
   { .size_bytes = 196609, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 256, 256, 3}}, .scale = {1, (const float[1]){0.003921568859368563}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conversion_0_output" },
   { .size_bytes = 262144, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 128, 128, 16}}, .scale = {1, (const float[1]){0.06288117170333862}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_1_output" },
   { .size_bytes = 270400, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 130, 130, 16}}, .scale = {1, (const float[1]){0.06288117170333862}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_2_pad_before_output" },
   { .size_bytes = 131072, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 64, 64, 32}}, .scale = {1, (const float[1]){0.09243304282426834}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_2_output" },
   { .size_bytes = 139392, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 66, 66, 32}}, .scale = {1, (const float[1]){0.09243304282426834}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_3_pad_before_output" },
   { .size_bytes = 65536, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 32, 32, 64}}, .scale = {1, (const float[1]){0.0652708038687706}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_3_output" },
   { .size_bytes = 73984, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 34, 34, 64}}, .scale = {1, (const float[1]){0.0652708038687706}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_4_pad_before_output" },
   { .size_bytes = 41472, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_FIRST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 128, 16, 16}}, .scale = {1, (const float[1]){0.06661328673362732}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_4_output" },
   { .size_bytes = 41472, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_FIRST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 128, 18, 18}}, .scale = {1, (const float[1]){0.06661328673362732}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_5_pad_before_output" },
   { .size_bytes = 32768, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 16, 16, 128}}, .scale = {1, (const float[1]){0.1389806717634201}}, .zeropoint = {1, (const int16_t[1]){2}}, .name = "conv2d_5_output" },
   { .size_bytes = 82944, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_FIRST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 256, 16, 16}}, .scale = {1, (const float[1]){0.08044284582138062}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_6_output" },
   { .size_bytes = 82944, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_FIRST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 256, 18, 18}}, .scale = {1, (const float[1]){0.08044284582138062}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_7_pad_before_output" },
   { .size_bytes = 65536, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 16, 16, 256}}, .scale = {1, (const float[1]){0.2299330234527588}}, .zeropoint = {1, (const int16_t[1]){2}}, .name = "conv2d_7_output" },
   { .size_bytes = 65536, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 16, 16, 256}}, .scale = {1, (const float[1]){0.05152946338057518}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_8_output" },
   { .size_bytes = 65536, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 16, 16, 256}}, .scale = {1, (const float[1]){0.05152946338057518}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "pool_9_output" },
   { .size_bytes = 82944, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 18, 18, 256}}, .scale = {1, (const float[1]){0.05152946338057518}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_10_pad_before_output" },
   { .size_bytes = 65536, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 16, 16, 256}}, .scale = {1, (const float[1]){0.1385585218667984}}, .zeropoint = {1, (const int16_t[1]){20}}, .name = "conv2d_10_output" },
   { .size_bytes = 41472, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_FIRST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 128, 16, 16}}, .scale = {1, (const float[1]){0.10237286239862442}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_11_output" },
   { .size_bytes = 41472, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_FIRST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 128, 18, 18}}, .scale = {1, (const float[1]){0.10237286239862442}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_12_pad_before_output" },
   { .size_bytes = 32768, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 16, 16, 128}}, .scale = {1, (const float[1]){0.13364781439304352}}, .zeropoint = {1, (const int16_t[1]){-28}}, .name = "conv2d_12_output" },
   { .size_bytes = 65536, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 16, 16, 256}}, .scale = {1, (const float[1]){0.030627276748418808}}, .zeropoint = {1, (const int16_t[1]){-128}}, .name = "conv2d_13_output" },
   { .size_bytes = 7680, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_S8, .shape = {4, (const int32_t[4]){1, 16, 16, 30}}, .scale = {1, (const float[1]){0.09362839162349701}}, .zeropoint = {1, (const int16_t[1]){50}}, .name = "conv2d_14_output" },
   { .size_bytes = 30720, .flags = (STAI_FLAG_HAS_BATCH|STAI_FLAG_CHANNEL_LAST), .format = STAI_FORMAT_FLOAT32, .shape = {4, (const int32_t[4]){1, 16, 16, 30}}, .scale = {0, NULL}, .zeropoint = {0, NULL}, .name = "conversion_15_output" }
  },
  .nodes = (const stai_node_details[23]){
    {.id = 0, .type = AI_LAYER_NL_TYPE, .input_tensors = {1, (const int32_t[1]){0}}, .output_tensors = {1, (const int32_t[1]){1}} }, /* conversion_0 */
    {.id = 1, .type = AI_LAYER_OPTIMIZED_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){1}}, .output_tensors = {1, (const int32_t[1]){2}} }, /* conv2d_1 */
    {.id = 2, .type = AI_LAYER_PAD_TYPE, .input_tensors = {1, (const int32_t[1]){2}}, .output_tensors = {1, (const int32_t[1]){3}} }, /* conv2d_2_pad_before */
    {.id = 2, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){3}}, .output_tensors = {1, (const int32_t[1]){4}} }, /* conv2d_2 */
    {.id = 3, .type = AI_LAYER_PAD_TYPE, .input_tensors = {1, (const int32_t[1]){4}}, .output_tensors = {1, (const int32_t[1]){5}} }, /* conv2d_3_pad_before */
    {.id = 3, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){5}}, .output_tensors = {1, (const int32_t[1]){6}} }, /* conv2d_3 */
    {.id = 4, .type = AI_LAYER_PAD_TYPE, .input_tensors = {1, (const int32_t[1]){6}}, .output_tensors = {1, (const int32_t[1]){7}} }, /* conv2d_4_pad_before */
    {.id = 4, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){7}}, .output_tensors = {1, (const int32_t[1]){8}} }, /* conv2d_4 */
    {.id = 5, .type = AI_LAYER_PAD_TYPE, .input_tensors = {1, (const int32_t[1]){8}}, .output_tensors = {1, (const int32_t[1]){9}} }, /* conv2d_5_pad_before */
    {.id = 5, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){9}}, .output_tensors = {1, (const int32_t[1]){10}} }, /* conv2d_5 */
    {.id = 6, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){10}}, .output_tensors = {1, (const int32_t[1]){11}} }, /* conv2d_6 */
    {.id = 7, .type = AI_LAYER_PAD_TYPE, .input_tensors = {1, (const int32_t[1]){11}}, .output_tensors = {1, (const int32_t[1]){12}} }, /* conv2d_7_pad_before */
    {.id = 7, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){12}}, .output_tensors = {1, (const int32_t[1]){13}} }, /* conv2d_7 */
    {.id = 8, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){13}}, .output_tensors = {1, (const int32_t[1]){14}} }, /* conv2d_8 */
    {.id = 9, .type = AI_LAYER_POOL_TYPE, .input_tensors = {1, (const int32_t[1]){14}}, .output_tensors = {1, (const int32_t[1]){15}} }, /* pool_9 */
    {.id = 10, .type = AI_LAYER_PAD_TYPE, .input_tensors = {1, (const int32_t[1]){15}}, .output_tensors = {1, (const int32_t[1]){16}} }, /* conv2d_10_pad_before */
    {.id = 10, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){16}}, .output_tensors = {1, (const int32_t[1]){17}} }, /* conv2d_10 */
    {.id = 11, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){17}}, .output_tensors = {1, (const int32_t[1]){18}} }, /* conv2d_11 */
    {.id = 12, .type = AI_LAYER_PAD_TYPE, .input_tensors = {1, (const int32_t[1]){18}}, .output_tensors = {1, (const int32_t[1]){19}} }, /* conv2d_12_pad_before */
    {.id = 12, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){19}}, .output_tensors = {1, (const int32_t[1]){20}} }, /* conv2d_12 */
    {.id = 13, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){20}}, .output_tensors = {1, (const int32_t[1]){21}} }, /* conv2d_13 */
    {.id = 14, .type = AI_LAYER_CONV2D_TYPE, .input_tensors = {1, (const int32_t[1]){21}}, .output_tensors = {1, (const int32_t[1]){22}} }, /* conv2d_14 */
    {.id = 15, .type = AI_LAYER_NL_TYPE, .input_tensors = {1, (const int32_t[1]){22}}, .output_tensors = {1, (const int32_t[1]){23}} } /* conversion_15 */
  },
  .n_nodes = 23
};
#endif

