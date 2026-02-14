/**
  ******************************************************************************
  * @file    network.c
  * @author  AST Embedded Analytics Research Platform
  * @date    2026-02-14T16:27:23+0000
  * @brief   AI Tool Automatic Code Generator for Embedded NN computing
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

#include "ai_lite_inspect.h"
#include "ai_platform_interface.h"
#include "layers.h"
#include "core_convert.h"
#include "network.h"
#include "network_details.h"
#include "network_data.h"
#include "stai_events.h"

#include "lite_operators.h"

#include "ai_lite_inspect.h"
/*****************************************************************************/
#define STAI_INTERNAL_API_MAJOR               (1)
#define STAI_INTERNAL_API_MINOR               (0)
#define STAI_INTERNAL_API_MICRO               (0)

#define STAI_MAGIC                            (0xB1C00100)

/*****************************************************************************/
#define _STAI_CONCAT_ARG(a, b)     a ## b
#define STAI_CONCAT(a, b)         _STAI_CONCAT_ARG(a, b)

/*!  STAI_CAST SECTION                       *********************************/
#define STAI_CAST(type, expr) \
  ((type)(expr))


/*****************************************************************************/
#define STAI_SIZE(_size) \
  ((stai_size)(_size))

/*****************************************************************************/
#define STAI_INIT_BUFFER(_flags, _size, _address) \
  { \
    .size = (_size), \
    .address = (uintptr_t)(_address), \
    .flags = (_flags), \
  }

#define STAI_INIT_TENSOR(_name, _flags, _fmt, _size_bytes, _shape, _scale, _zeropoint) \
  { \
    .size_bytes = (_size_bytes), \
    .flags = (_flags), \
    .format = (stai_format)(_fmt), \
    .shape = STAI_PACK(_shape), \
    .scale = STAI_PACK(_scale), \
    .zeropoint = STAI_PACK(_zeropoint), \
    .name = (_name) \
  }

#define STAI_INIT_ARRAY(_size, _ptr) \
  { .size = STAI_SIZE(_size), .data = STAI_PACK(_ptr) }


#define STAI_CAST_ARRAY(_type, _size, _ptr) \
  { .size = STAI_SIZE(_size), .data = (_type)STAI_PACK(_ptr) }


#define STAI_DECLARE_ARRAY(_type, _size, ...) \
  { .size = STAI_SIZE(_size), .data = (_type[_size]) { STAI_PACK(__VA_ARGS__) } }


#define STAI_EMPTY_ARRAY() \
  { .size = 0, .data = NULL }


#define STAI_INIT_VERSION(_major, _minor, _micro) \
  { .major = (_major), .minor = (_minor), .micro = (_micro), .reserved = 0x0 }

/*****************************************************************************/
/**  Getters and setters  **/

#define STAI_GET_ARRAY_SIZE(nd_array) \
  (nd_array.size)


#define STAI_GET_ARRAY_ELEM(nd_array, pos) \
  (nd_array.data[(pos)])

#define _STAI_SET_ERROR(net_ctx, cond, value, exit) { \
  if (!(net_ctx)) { return STAI_ERROR_NETWORK_INVALID_CONTEXT_HANDLE; } \
  if (((uintptr_t)net_ctx) & (_STAI_CONTEXT_ALIGNMENT-1)) { return STAI_ERROR_NETWORK_INVALID_CONTEXT_ALIGNMENT; } \
  if (((value) >= STAI_ERROR_GENERIC) && (cond)) { \
    if ((net_ctx)->_return_code == STAI_SUCCESS) { \
      (net_ctx)->_return_code = (value); \
    } \
    return (exit); \
  } \
}

/*****************************************************************************/
/* TODO REMOVE THESE TWO MACROS */
#define STAI_EVENT_NODE_START_CB
#define STAI_EVENT_NODE_STOP_CB

#ifdef STAI_EVENT_NODE_START_CB
#ifndef _STAI_NETWORK_EVENT_NODE_START_CB
  #define _STAI_NETWORK_EVENT_NODE_START_CB(_node_id, _buffers_size, ...) \
  if (net_ctx->_callback) { \
    const stai_event_node_start_stop _start_event = { \
      .node_id=(_node_id), \
      .buffers={ \
        .size=(_buffers_size), \
        .data=(stai_ptr const*)(const stai_ptr[_buffers_size])STAI_PACK(__VA_ARGS__) \
      } \
    }; \
    net_ctx->_callback(net_ctx->_callback_cookie, STAI_EVENT_NODE_START, (const void*)&_start_event); \
  }
#endif
#else
  #define _STAI_NETWORK_EVENT_NODE_START_CB(_node_id, _buffers_size, ...) \
    do { /* _STAI_NETWORK_EVENT_NODE_START_CB() */ } while(0);
#endif      /* STAI_EVENT_NODE_START_CB */

#ifdef STAI_EVENT_NODE_STOP_CB
#ifndef _STAI_NETWORK_EVENT_NODE_STOP_CB
  #define _STAI_NETWORK_EVENT_NODE_STOP_CB(_node_id, _buffers_size, ...) \
  if (net_ctx->_callback) { \
    const stai_event_node_start_stop _stop_event = { \
      .node_id=(_node_id), \
      .buffers={ \
        .size=(_buffers_size), \
        .data=(stai_ptr const*)(stai_ptr[_buffers_size])STAI_PACK(__VA_ARGS__) \
      } \
    }; \
    net_ctx->_callback(net_ctx->_callback_cookie, STAI_EVENT_NODE_STOP, (const void*)&_stop_event); \
  }
#endif
#else
  #define _STAI_NETWORK_EVENT_NODE_STOP_CB(_node_id, _buffers_size, ...) \
    do { /* _STAI_NETWORK_EVENT_NODE_STOP_CB() */ } while(0);
#endif      /* STAI_EVENT_NODE_STOP_CB */


/*****************************************************************************/
#define _STAI_NETWORK_MODEL_SIGNATURE     "0x802a0356833e0d95123d41515c7f5c20"
#define _STAI_NETWORK_DATETIME            "2026-02-14T16:27:23+0000"
#define _STAI_NETWORK_COMPILE_DATETIME    __DATE__ " " __TIME__

#define _STAI_CONTEXT_ALIGNMENT        STAI_NETWORK_CONTEXT_ALIGNMENT

/*****************************************************************************/
#define g_network_activations_1     (NULL)




#if defined(HAVE_NETWORK_INFO)
/*****************************************************************************/
static const stai_network_info g_network_info = {
  .model_signature = _STAI_NETWORK_MODEL_SIGNATURE,
  .c_compile_datetime = _STAI_NETWORK_COMPILE_DATETIME,
  .c_model_name = STAI_NETWORK_MODEL_NAME,
  .c_model_datetime = _STAI_NETWORK_DATETIME,
  .c_model_signature = 0x0,
  .runtime_version = STAI_INIT_VERSION(11, 0, 0),
  .tool_version = STAI_INIT_VERSION(3, 0, 0),
  .api_version = STAI_INIT_VERSION(1, 0, 0),
  .n_macc = STAI_NETWORK_MACC_NUM,
  .n_nodes = STAI_NETWORK_NODES_NUM,
  .flags = STAI_NETWORK_FLAGS,
  .n_inputs = STAI_NETWORK_IN_NUM,
  .n_outputs = STAI_NETWORK_OUT_NUM,
  .n_activations = STAI_NETWORK_ACTIVATIONS_NUM,
  .n_weights = STAI_NETWORK_WEIGHTS_NUM,
  .n_states = STAI_NETWORK_STATES_NUM,
  .inputs = (stai_tensor[STAI_NETWORK_IN_NUM]) {
    STAI_INIT_TENSOR(
      STAI_NETWORK_IN_1_NAME,
      STAI_NETWORK_IN_1_FLAGS,
      STAI_NETWORK_IN_1_FORMAT,
      STAI_NETWORK_IN_1_SIZE_BYTES,
      STAI_DECLARE_ARRAY(int32_t, 4, 1, 256, 256, 3),
      STAI_DECLARE_ARRAY(float, 1, 0.003921568859368563f),
      STAI_DECLARE_ARRAY(int16_t, 1, 0)),
    },
    .outputs = (stai_tensor[STAI_NETWORK_OUT_NUM]) {
    STAI_INIT_TENSOR(
      STAI_NETWORK_OUT_1_NAME,
      STAI_NETWORK_OUT_1_FLAGS,
      STAI_NETWORK_OUT_1_FORMAT,
      STAI_NETWORK_OUT_1_SIZE_BYTES,
      STAI_DECLARE_ARRAY(int32_t, 4, 1, 16, 16, 30),
      STAI_EMPTY_ARRAY(),
      STAI_EMPTY_ARRAY()),
    },
  .activations = (stai_tensor[STAI_NETWORK_ACTIVATIONS_NUM]) {
    STAI_INIT_TENSOR(
      (NULL),
      STAI_NETWORK_ACTIVATION_1_FLAGS,
      STAI_FORMAT_U8,
      STAI_NETWORK_ACTIVATION_1_SIZE_BYTES,
      STAI_DECLARE_ARRAY(int32_t, 1, 459952),
      STAI_EMPTY_ARRAY(),
      STAI_EMPTY_ARRAY()),
    },
  .weights = (stai_tensor[STAI_NETWORK_WEIGHTS_NUM]) {
    STAI_INIT_TENSOR(
      (NULL),
      STAI_NETWORK_WEIGHT_1_FLAGS,
      STAI_FORMAT_U8,
      STAI_NETWORK_WEIGHT_1_SIZE_BYTES,
      STAI_DECLARE_ARRAY(int32_t, 1, 283368),
      STAI_EMPTY_ARRAY(),
      STAI_EMPTY_ARRAY()),
    },

  .states = NULL
};
#endif

#define _STAI_CONTEXT_ACQUIRE(_net_ctx, _net_handle) \
  _stai_network_context* _net_ctx = (_stai_network_context*)(_net_handle); \
  STAI_ASSERT(_net_ctx != NULL) \
  _STAI_SET_ERROR(_net_ctx, _net_ctx->_magic != STAI_MAGIC, \
                  STAI_ERROR_NETWORK_INVALID_CONTEXT_HANDLE, _net_ctx->_return_code)


/*****************************************************************************/
static
void _stai_network_check(_stai_network_context* net_ctx)
{
  stai_size idx;

// Check activations status
  for (idx=0; idx<STAI_NETWORK_ACTIVATIONS_NUM; idx++) {
    if (net_ctx->_activations[idx] == NULL) break;
  }
  net_ctx->_flags |= (idx == STAI_NETWORK_ACTIVATIONS_NUM) ? STAI_FLAG_ACTIVATIONS : STAI_FLAG_NONE;
// Check inputs status
  for (idx=0; idx<STAI_NETWORK_IN_NUM; idx++) {
    if (net_ctx->_inputs[idx] == NULL) break;
  }
  net_ctx->_flags |= (idx == STAI_NETWORK_IN_NUM) ? STAI_FLAG_INPUTS : STAI_FLAG_NONE;

  // Check outputs status
  for (idx=0; idx<STAI_NETWORK_OUT_NUM; idx++) {
    if (net_ctx->_outputs[idx] == NULL) break;
  }
  net_ctx->_flags |= (idx == STAI_NETWORK_OUT_NUM) ? STAI_FLAG_OUTPUTS : STAI_FLAG_NONE;

// Check weights status
  for (idx=0; idx<STAI_NETWORK_WEIGHTS_NUM; idx++) {
    if (net_ctx->_weights[idx] == NULL) break;
  }
  net_ctx->_flags |= (idx == STAI_NETWORK_WEIGHTS_NUM) ? STAI_FLAG_WEIGHTS : STAI_FLAG_NONE;
STAI_PRINT("  [_stai_network_check] flags: 0x%08x\n", net_ctx->_flags)
}


/*****************************************************************************/
STAI_API_ENTRY
stai_return_code stai_network_init(
  stai_network* network)
{
  /* Memory where to store internal context is provided by applications as a raw byte buffer */
  _stai_network_context* net_ctx = (_stai_network_context*)(network);
  net_ctx->_return_code = STAI_SUCCESS;
  STAI_PRINT("[Entering Network Init] network(%p) context_size(%d)\n", net_ctx, (int32_t)sizeof(_stai_network_context))

  _STAI_SET_ERROR(net_ctx, STAI_NETWORK_CONTEXT_SIZE != sizeof(_stai_network_context),
                 STAI_ERROR_NETWORK_INVALID_CONTEXT_SIZE, net_ctx->_return_code)

  {
    const _stai_network_context _network_context = {
      ._magic = STAI_MAGIC,
      ._signature = STAI_NETWORK_MODEL_SIGNATURE,
      ._flags = STAI_NETWORK_FLAGS,
      ._return_code = STAI_SUCCESS,
      ._callback = NULL,
      ._callback_cookie = NULL,
      ._activations = {
      (stai_ptr)g_network_activations_1
      },
      ._weights = {
      (stai_ptr)g_network_weights_array
      },
      ._inputs = {
    NULL},
      ._outputs = {
    NULL},
    };

    // Deep copy of internal context to opaque buffer provided by app
    *net_ctx = _network_context;

    _stai_network_check(net_ctx);
  }

  return net_ctx->_return_code;
}


STAI_API_ENTRY
stai_return_code stai_network_deinit(
  stai_network* network)
{
  _STAI_CONTEXT_ACQUIRE(net_ctx, network)

  /*  Reset flags to initial state  */
  net_ctx->_flags = STAI_NETWORK_FLAGS;
  return net_ctx->_return_code;
}

/*****************************************************************************/



/* Int quant #0 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(conv2d_4_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.06661328673362732f),
    AI_PACK_INTQ_ZP(-128)))

/* Int quant #1 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(conv2d_5_pad_before_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.06661328673362732f),
    AI_PACK_INTQ_ZP(-128)))

/* Int quant #2 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(conv2d_6_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.08044284582138062f),
    AI_PACK_INTQ_ZP(-128)))

/* Int quant #3 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(conv2d_7_pad_before_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.08044284582138062f),
    AI_PACK_INTQ_ZP(-128)))

/* Int quant #4 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(conv2d_11_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.10237286239862442f),
    AI_PACK_INTQ_ZP(-128)))

/* Int quant #5 */
AI_INTQ_INFO_LIST_OBJ_DECLARE(conv2d_12_pad_before_output_array_intq, AI_STATIC,
  AI_BUFFER_META_FLAG_SCALE_FLOAT|AI_BUFFER_META_FLAG_ZEROPOINT_S8, 1,
  AI_PACK_INTQ_INFO(
    AI_PACK_INTQ_SCALE(0.10237286239862442f),
    AI_PACK_INTQ_ZP(-128)))



/* Array#0 */
AI_ARRAY_OBJ_DECLARE(
  conv2d_4_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 41472, AI_STATIC)

/* Array#1 */
AI_ARRAY_OBJ_DECLARE(
  conv2d_5_pad_before_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 41472, AI_STATIC)

/* Array#2 */
AI_ARRAY_OBJ_DECLARE(
  conv2d_6_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 82944, AI_STATIC)

/* Array#3 */
AI_ARRAY_OBJ_DECLARE(
  conv2d_7_pad_before_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 82944, AI_STATIC)

/* Array#4 */
AI_ARRAY_OBJ_DECLARE(
  conv2d_11_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 41472, AI_STATIC)

/* Array#5 */
AI_ARRAY_OBJ_DECLARE(
  conv2d_12_pad_before_output_array, AI_ARRAY_FORMAT_S8,
  NULL, NULL, 41472, AI_STATIC)



/* Tensor #0 */
AI_TENSOR_OBJ_DECLARE(
  conv2d_4_output, AI_STATIC,
  37, 0x1,
  AI_SHAPE_INIT(4, 1, 128, 16, 16), AI_STRIDE_INIT(4, 1, 16*16, 1, 16),
  1, &conv2d_4_output_array, &conv2d_4_output_array_intq)

/* Tensor #1 */
AI_TENSOR_OBJ_DECLARE(
  conv2d_5_pad_before_output, AI_STATIC,
  43, 0x1,
  AI_SHAPE_INIT(4, 1, 128, 18, 18), AI_STRIDE_INIT(4, 1, 18*18, 1, 18),
  1, &conv2d_5_pad_before_output_array, &conv2d_5_pad_before_output_array_intq)

/* Tensor #2 */
AI_TENSOR_OBJ_DECLARE(
  conv2d_6_output, AI_STATIC,
  47, 0x1,
  AI_SHAPE_INIT(4, 1, 256, 16, 16), AI_STRIDE_INIT(4, 1, 16*16, 1, 16),
  1, &conv2d_6_output_array, &conv2d_6_output_array_intq)

/* Tensor #3 */
AI_TENSOR_OBJ_DECLARE(
  conv2d_7_pad_before_output, AI_STATIC,
  52, 0x1,
  AI_SHAPE_INIT(4, 1, 256, 18, 18), AI_STRIDE_INIT(4, 1, 18*18, 1, 18),
  1, &conv2d_7_pad_before_output_array, &conv2d_7_pad_before_output_array_intq)

/* Tensor #4 */
AI_TENSOR_OBJ_DECLARE(
  conv2d_11_output, AI_STATIC,
  6, 0x1,
  AI_SHAPE_INIT(4, 1, 128, 16, 16), AI_STRIDE_INIT(4, 1, 16*16, 1, 16),
  1, &conv2d_11_output_array, &conv2d_11_output_array_intq)

/* Tensor #5 */
AI_TENSOR_OBJ_DECLARE(
  conv2d_12_pad_before_output, AI_STATIC,
  11, 0x1,
  AI_SHAPE_INIT(4, 1, 128, 18, 18), AI_STRIDE_INIT(4, 1, 18*18, 1, 18),
  1, &conv2d_12_pad_before_output_array, &conv2d_12_pad_before_output_array_intq)



AI_STATIC_CONST ai_i8 conv2d_5_pad_before_value_data[] = { -128 };
AI_ARRAY_OBJ_DECLARE(
    conv2d_5_pad_before_value, AI_ARRAY_FORMAT_S8,
    conv2d_5_pad_before_value_data, conv2d_5_pad_before_value_data, 1, AI_STATIC_CONST)
AI_TENSOR_CHAIN_OBJ_DECLARE(
  conv2d_5_pad_before_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &conv2d_4_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &conv2d_5_pad_before_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  conv2d_5_pad_before_layer, 5,
  PAD_TYPE, 0x0, NULL,
  pad, forward_pad,
  &conv2d_5_pad_before_chain,
  NULL, &conv2d_5_pad_before_layer, AI_STATIC, 
  .value = &conv2d_5_pad_before_value, 
  .mode = AI_PAD_8BIT_CH1ST_CONSTANT, 
  .pads = AI_SHAPE_INIT(4, 1, 1, 1, 1), 
)


AI_STATIC_CONST ai_i8 conv2d_7_pad_before_value_data[] = { -128 };
AI_ARRAY_OBJ_DECLARE(
    conv2d_7_pad_before_value, AI_ARRAY_FORMAT_S8,
    conv2d_7_pad_before_value_data, conv2d_7_pad_before_value_data, 1, AI_STATIC_CONST)
AI_TENSOR_CHAIN_OBJ_DECLARE(
  conv2d_7_pad_before_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &conv2d_6_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &conv2d_7_pad_before_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  conv2d_7_pad_before_layer, 7,
  PAD_TYPE, 0x0, NULL,
  pad, forward_pad,
  &conv2d_7_pad_before_chain,
  NULL, &conv2d_7_pad_before_layer, AI_STATIC, 
  .value = &conv2d_7_pad_before_value, 
  .mode = AI_PAD_8BIT_CH1ST_CONSTANT, 
  .pads = AI_SHAPE_INIT(4, 1, 1, 1, 1), 
)


AI_STATIC_CONST ai_i8 conv2d_12_pad_before_value_data[] = { -128 };
AI_ARRAY_OBJ_DECLARE(
    conv2d_12_pad_before_value, AI_ARRAY_FORMAT_S8,
    conv2d_12_pad_before_value_data, conv2d_12_pad_before_value_data, 1, AI_STATIC_CONST)
AI_TENSOR_CHAIN_OBJ_DECLARE(
  conv2d_12_pad_before_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &conv2d_11_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &conv2d_12_pad_before_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  conv2d_12_pad_before_layer, 12,
  PAD_TYPE, 0x0, NULL,
  pad, forward_pad,
  &conv2d_12_pad_before_chain,
  NULL, &conv2d_12_pad_before_layer, AI_STATIC, 
  .value = &conv2d_12_pad_before_value, 
  .mode = AI_PAD_8BIT_CH1ST_CONSTANT, 
  .pads = AI_SHAPE_INIT(4, 1, 1, 1, 1), 
)
/**  Hybrid layers declarations section  *************************************/
void forward_lite_conv2d_5_pad_before(_stai_network_context* net_ctx)
{
  conv2d_4_output_array.data = AI_PTR(net_ctx->_activations[0] + 9216);
  conv2d_4_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 9216);
  conv2d_5_pad_before_output_array.data = AI_PTR(net_ctx->_activations[0] + 9216);
  conv2d_5_pad_before_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 9216);
  _STAI_NETWORK_EVENT_NODE_START_CB(5, 1, { conv2d_4_output.data->data});
  forward_pad(&conv2d_5_pad_before_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(5, 1, { conv2d_5_pad_before_output.data->data});
}
void forward_lite_conv2d_7_pad_before(_stai_network_context* net_ctx)
{
  conv2d_6_output_array.data = AI_PTR(net_ctx->_activations[0] + 83456);
  conv2d_6_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 83456);
  conv2d_7_pad_before_output_array.data = AI_PTR(net_ctx->_activations[0] + 83456);
  conv2d_7_pad_before_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 83456);
  _STAI_NETWORK_EVENT_NODE_START_CB(7, 1, { conv2d_6_output.data->data});
  forward_pad(&conv2d_7_pad_before_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(7, 1, { conv2d_7_pad_before_output.data->data});
}
void forward_lite_conv2d_12_pad_before(_stai_network_context* net_ctx)
{
  conv2d_11_output_array.data = AI_PTR(net_ctx->_activations[0] + 2304);
  conv2d_11_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 2304);
  conv2d_12_pad_before_output_array.data = AI_PTR(net_ctx->_activations[0] + 2304);
  conv2d_12_pad_before_output_array.data_start = AI_PTR(net_ctx->_activations[0] + 2304);
  _STAI_NETWORK_EVENT_NODE_START_CB(12, 1, { conv2d_11_output.data->data});
  forward_pad(&conv2d_12_pad_before_layer);
  _STAI_NETWORK_EVENT_NODE_STOP_CB(12, 1, { conv2d_12_pad_before_output.data->data});
}

/*****************************************************************************/


static const ai_u32 conversion_0_t_in_0_shape_h_w_ch_d_prod_const_u32 = 196608;
static const ai_u8 conversion_0_t_in_0_fmt_zero_const_u8 = 0;
static const ai_i8 conversion_0_t_out_0_fmt_zero_const_s8 = -128;

static const ai_u16 conv2d_1_t_in_0_shape_w_const_u16 = 256;
static const ai_u16 conv2d_1_t_out_0_shape_ch_const_u16 = 16;
static const ai_u16 conv2d_1_t_weight_0_shape_w_const_u16 = 3;
static const ai_i32 conv2d_1_l_pad_W_0_const_s32 = 0;
static const ai_u16 conv2d_1_l_stride_0_const_u16 = 2;
static const ai_i8 conv2d_1_t_in_0_fmt_zero_const_s8 = -128;
static const ai_i8 conv2d_1_t_out_0_fmt_zero_const_s8 = -128;
static const ai_float conv2d_1_t_in_0_fmt_scale_const_f32 = 0.003921568859368563f;
static const ai_float conv2d_1_t_out_0_fmt_scale_const_f32 = 0.06288117170333862f;
static const ai_float conv2d_1_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.026749001815915108f, 0.012035850435495377f, 0.009460780769586563f, 0.006854751147329807f, 0.0122905932366848f, 0.010176117531955242f, 0.01886916719377041f, 0.017170634120702744f, 0.013513305224478245f, 0.015175359323620796f, 0.019227229058742523f, 0.01601346582174301f, 0.014069396071135998f, 0.017272749915719032f, 0.03866952657699585f, 0.028983648866415024f);
static const ai_layer_format_type conv2d_1_l_out_ch_format_const_layer_format_type = AI_LAYER_FORMAT_CHANNEL_LAST_VALID;
static const ai_u16 conv2d_1_t_out_0_shape_w_const_u16 = 128;

static const ai_i8 conv2d_2_pad_before_v_pad_constant_value_const_s8[] = LITE_ARRAY_VALUES(-128);
static const ai_i16 conv2d_2_pad_before_t_in_0_fmt_bitsize_const_s16 = 8;
static const ai_u32 conv2d_2_pad_before_t_in_0_shape_h_const_u32 = 128;

static const ai_u16 conv2d_2_t_in_0_shape_w_const_u16 = 130;
static const ai_u16 conv2d_2_t_in_0_shape_h_const_u16 = 130;
static const ai_u16 conv2d_2_t_in_0_shape_ch_const_u16 = 16;
static const ai_u16 conv2d_2_t_out_0_shape_ch_const_u16 = 32;
static const ai_u16 conv2d_2_t_weight_0_shape_w_const_u16 = 3;
static const ai_u16 conv2d_2_t_weight_0_shape_h_const_u16 = 3;
static const ai_u16 conv2d_2_l_stride_1_const_u16 = 2;
static const ai_u16 conv2d_2_l_stride_0_const_u16 = 2;
static const ai_i32 conv2d_2_l_pad_W_0_const_s32 = 0;
static const ai_i32 conv2d_2_l_pad_H_0_const_s32 = 0;
static const ai_i8 conv2d_2_t_in_0_fmt_zero_const_s8 = -128;
static const ai_i8 conv2d_2_t_out_0_fmt_zero_const_s8 = -128;
static const ai_float conv2d_2_t_in_0_fmt_scale_const_f32 = 0.06288117170333862f;
static const ai_float conv2d_2_t_out_0_fmt_scale_const_f32 = 0.09243304282426834f;
static const ai_float conv2d_2_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.004553643986582756f, 0.0028853719122707844f, 0.005621688440442085f, 0.003032113192602992f, 0.002999222371727228f, 0.004017090890556574f, 0.0025209824088960886f, 0.002253615530207753f, 0.0032724866177886724f, 0.007669519167393446f, 0.00392058165743947f, 0.005458046216517687f, 0.005832377355545759f, 0.0051232632249593735f, 0.0037615816108882427f, 0.004050957038998604f, 0.002242343733087182f, 0.007658456917852163f, 0.004812279716134071f, 0.005323323421180248f, 0.0050875348970294f, 0.004220929462462664f, 0.003601189237087965f, 0.004999909084290266f, 0.00543710682541132f, 0.0042589628137648106f, 0.007107770536094904f, 0.0029437795747071505f, 0.0020109093748033047f, 0.005302933510392904f, 0.006351334974169731f, 0.005854404531419277f);
static const ai_layer_format_type conv2d_2_l_out_ch_format_const_layer_format_type = AI_LAYER_FORMAT_CHANNEL_LAST_VALID;
static const ai_u16 conv2d_2_t_out_0_shape_w_const_u16 = 64;
static const ai_u16 conv2d_2_t_out_0_shape_h_const_u16 = 64;

static const ai_i8 conv2d_3_pad_before_v_pad_constant_value_const_s8[] = LITE_ARRAY_VALUES(-128);
static const ai_i16 conv2d_3_pad_before_t_in_0_fmt_bitsize_const_s16 = 8;
static const ai_u32 conv2d_3_pad_before_t_in_0_shape_h_const_u32 = 64;

static const ai_u16 conv2d_3_t_in_0_shape_w_const_u16 = 66;
static const ai_u16 conv2d_3_t_in_0_shape_h_const_u16 = 66;
static const ai_u16 conv2d_3_t_in_0_shape_ch_const_u16 = 32;
static const ai_u16 conv2d_3_t_out_0_shape_ch_const_u16 = 64;
static const ai_u16 conv2d_3_t_weight_0_shape_w_const_u16 = 3;
static const ai_u16 conv2d_3_t_weight_0_shape_h_const_u16 = 3;
static const ai_u16 conv2d_3_l_stride_1_const_u16 = 2;
static const ai_u16 conv2d_3_l_stride_0_const_u16 = 2;
static const ai_i8 conv2d_3_t_in_0_fmt_zero_const_s8 = -128;
static const ai_i8 conv2d_3_t_out_0_fmt_zero_const_s8 = -128;
static const ai_float conv2d_3_t_in_0_fmt_scale_const_f32 = 0.09243304282426834f;
static const ai_float conv2d_3_t_out_0_fmt_scale_const_f32 = 0.0652708038687706f;
static const ai_float conv2d_3_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.0028841954190284014f, 0.001444240682758391f, 0.002222528913989663f, 0.0017500030808150768f, 0.0026375828310847282f, 0.005593453533947468f, 0.001715152757242322f, 0.00223310268484056f, 0.0019703798461705446f, 0.001736319507472217f, 0.003002672456204891f, 0.002265410963445902f, 0.0022210562601685524f, 0.00310134282335639f, 0.0040193041786551476f, 0.0028568808920681477f, 0.0028230007737874985f, 0.002888919087126851f, 0.0028149299323558807f, 0.0028372113592922688f, 0.002467020181939006f, 0.004645270295441151f, 0.001663869246840477f, 0.0025363450404256582f, 0.0019959243945777416f, 0.0024992497637867928f, 0.0011165626347064972f, 0.00455405144020915f, 0.004784704186022282f, 0.0022458930034190416f, 0.0015170684782788157f, 0.0014448256697505713f, 0.0016631262842565775f, 0.0043156822212040424f, 0.0014498477103188634f, 0.0033870830666273832f, 0.00140820920933038f, 0.005143716931343079f, 0.0026258837897330523f, 0.002444293349981308f, 0.002911279210820794f, 0.001999232452362776f, 0.002144078491255641f, 0.0023900093510746956f, 0.001953792292624712f, 0.001408045762218535f, 0.0035807958338409662f, 0.0017575760139152408f, 0.0032400095369666815f, 0.0027787289582192898f, 0.0032588085159659386f, 0.002942394930869341f, 0.004683075938373804f, 0.0019037150777876377f, 0.0020269437227398157f, 0.002422603080049157f, 0.0024204824585467577f, 0.004623116459697485f, 0.0034381297882646322f, 0.005939226597547531f, 0.0031805371399968863f, 0.0026274453848600388f, 0.0017611841903999448f, 0.001874467241577804f);
static const ai_layer_format_type conv2d_3_l_out_ch_format_const_layer_format_type = AI_LAYER_FORMAT_CHANNEL_LAST_VALID;
static const ai_u16 conv2d_3_t_out_0_shape_w_const_u16 = 32;
static const ai_u16 conv2d_3_t_out_0_shape_h_const_u16 = 32;

static const ai_i8 conv2d_4_pad_before_v_pad_constant_value_const_s8[] = LITE_ARRAY_VALUES(-128);
static const ai_i16 conv2d_4_pad_before_t_in_0_fmt_bitsize_const_s16 = 8;
static const ai_u32 conv2d_4_pad_before_t_in_0_shape_h_const_u32 = 32;

static const ai_u16 conv2d_4_t_in_0_shape_w_const_u16 = 34;
static const ai_u16 conv2d_4_t_in_0_shape_h_const_u16 = 34;
static const ai_u16 conv2d_4_t_in_0_shape_ch_const_u16 = 64;
static const ai_u16 conv2d_4_t_out_0_shape_ch_const_u16 = 128;
static const ai_u16 conv2d_4_t_weight_0_shape_w_const_u16 = 3;
static const ai_u16 conv2d_4_t_weight_0_shape_h_const_u16 = 3;
static const ai_u16 conv2d_4_l_stride_1_const_u16 = 2;
static const ai_u16 conv2d_4_l_stride_0_const_u16 = 2;
static const ai_i8 conv2d_4_t_in_0_fmt_zero_const_s8 = -128;
static const ai_i8 conv2d_4_t_out_0_fmt_zero_const_s8 = -128;
static const ai_float conv2d_4_t_in_0_fmt_scale_const_f32 = 0.0652708038687706f;
static const ai_float conv2d_4_t_out_0_fmt_scale_const_f32 = 0.06661328673362732f;
static const ai_float conv2d_4_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.0006076590507291257f, 0.001010208623483777f, 0.0008804024546407163f, 0.0017506644362583756f, 0.00358013017103076f, 0.0008811148582026362f, 0.002445652149617672f, 0.003208636771887541f, 0.005028164014220238f, 0.002591969445347786f, 0.0013363795587792993f, 0.0021405674051493406f, 0.0012255454203113914f, 0.0009957061847671866f, 0.002119584707543254f, 0.0019247995223850012f, 0.002068476751446724f, 0.002587186172604561f, 0.0024098516441881657f, 0.0008615718106739223f, 0.004579267464578152f, 0.00012764858547598124f, 0.001564157078973949f, 0.0008213640539906919f, 0.0013175451895222068f, 0.0021012078505009413f, 0.003501927712932229f, 0.0007123047835193574f, 0.002705912571400404f, 0.0020166446920484304f, 0.0025816664565354586f, 0.0012089378433302045f, 0.002318113809451461f, 0.0008288083481602371f, 0.0031303074210882187f, 0.002523549599573016f, 0.001582671538926661f, 0.004928987473249435f, 0.0009224116220138967f, 0.0019570544827729464f, 0.0014841866213828325f, 0.0041410126723349094f, 0.0010817479342222214f, 0.0018509755609557033f, 0.0001454686134820804f, 0.0022090112324804068f, 0.0009827889734879136f, 0.0016319738933816552f, 0.0017266485374420881f, 0.0009245495893992484f, 0.004709337372332811f, 0.0026731460820883512f, 0.0033521002624183893f, 0.0024283777456730604f, 0.0037719120737165213f, 0.0017388953128829598f, 0.0010277320398017764f, 0.0014978342223912477f, 0.0007227608584798872f, 0.000910037022549659f, 0.004084746353328228f, 0.0007731410441920161f, 0.0012036524713039398f, 0.0011426886776462197f, 0.00609743595123291f, 0.0032370060216635466f, 0.0014430497540161014f, 0.0008341331849806011f, 0.002981774043291807f, 0.003571824636310339f, 0.0016247190069407225f, 0.002173534594476223f, 0.0012701712548732758f, 0.0016415761783719063f, 0.0022877566516399384f, 0.0010777459247037768f, 0.0006390729686245322f, 0.0015240785432979465f, 0.0007617779192514718f, 0.003779167775064707f, 0.0009241604129783809f, 0.003637343645095825f, 0.002146222162991762f, 0.0014423263492062688f, 0.0014325015945360065f, 0.002442652825266123f, 0.002304424298927188f, 0.00014098314568400383f, 0.0010504750534892082f, 0.002420350443571806f, 0.0013538503553718328f, 0.0016592255560681224f, 0.00672271940857172f, 0.0034442238975316286f, 0.0007777811260893941f, 0.0005573265952989459f, 0.004157472867518663f, 0.000879168976098299f, 0.0022024833597242832f, 0.0019840190652757883f, 0.0030275541357696056f, 0.0007222922868095338f, 0.002770428080111742f, 0.0011147309560328722f, 0.0019027076195925474f, 0.0013476888416334987f, 0.0024172868579626083f, 0.0018077451968565583f, 0.00011559508857317269f, 0.002359194215387106f, 0.006589994765818119f, 0.00470735365524888f, 0.0007422855123877525f, 0.0008092024363577366f, 0.0009804237633943558f, 0.0023962524719536304f, 0.0006128578097559512f, 0.0013975185574963689f, 0.002809701021760702f, 0.0027621109038591385f, 0.0012565061915665865f, 0.0009011264774017036f, 0.002600668929517269f, 9.732890612212941e-05f, 0.00017621301230974495f, 0.001493258634582162f, 0.0013484423980116844f, 0.0033720016945153475f);
static const ai_layer_format_type conv2d_4_l_out_ch_format_const_layer_format_type = AI_LAYER_FORMAT_CHANNEL_FIRST_SAME;
static const ai_u16 conv2d_4_t_out_0_shape_w_const_u16 = 16;
static const ai_u16 conv2d_4_t_out_0_shape_h_const_u16 = 16;


static const ai_u16 conv2d_5_t_in_0_shape_w_const_u16 = 18;
static const ai_u16 conv2d_5_t_in_0_shape_h_const_u16 = 18;
static const ai_u16 conv2d_5_t_in_0_shape_ch_const_u16 = 128;
static const ai_u16 conv2d_5_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_5_l_stride_0_const_u16 = 1;
static const ai_i8 conv2d_5_t_in_0_fmt_zero_const_s8 = -128;
static const ai_i8 conv2d_5_t_out_0_fmt_zero_const_s8 = 2;
static const ai_float conv2d_5_t_in_0_fmt_scale_const_f32 = 0.06661328673362732f;
static const ai_float conv2d_5_t_out_0_fmt_scale_const_f32 = 0.1389806717634201f;
static const ai_float conv2d_5_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.03416107967495918f, 0.009382904507219791f, 0.012822376564145088f, 0.024228958413004875f, 0.009023405611515045f, 0.010982274077832699f, 0.008652177639305592f, 0.009280805476009846f, 0.011448225006461143f, 0.011132503859698772f, 0.0168959628790617f, 0.00712655670940876f, 0.016865931451320648f, 0.02387986145913601f, 0.011468888260424137f, 0.008525784127414227f, 0.007072707172483206f, 0.004305918700993061f, 0.01473851315677166f, 0.01682247780263424f, 0.0053986008279025555f, 0.026589244604110718f, 0.01903146132826805f, 0.013598499819636345f, 0.021066348999738693f, 0.010816987603902817f, 0.002649917732924223f, 0.009163353592157364f, 0.0033834197092801332f, 0.007481243461370468f, 0.011960791423916817f, 0.019955476745963097f, 0.011661992408335209f, 0.012424079701304436f, 0.003424790222197771f, 0.01032967772334814f, 0.008029665797948837f, 0.004983198828995228f, 0.020851681008934975f, 0.01902833953499794f, 0.012888249941170216f, 0.003752684686332941f, 0.011041463352739811f, 0.014952549710869789f, 0.022699307650327682f, 0.012378321960568428f, 0.016287866979837418f, 0.010238000191748142f, 0.010835730470716953f, 0.046182990074157715f, 0.008869494311511517f, 0.007683910895138979f, 0.007708802819252014f, 0.008044511079788208f, 0.0056862663477659225f, 0.008712706156075f, 0.012650538235902786f, 0.015083554200828075f, 0.014947183430194855f, 0.019867561757564545f, 0.008208897896111012f, 0.01030091941356659f, 0.01006696280092001f, 0.010447642765939236f, 0.002621674444526434f, 0.0055806078016757965f, 0.030287235975265503f, 0.025569137185811996f, 0.008354702964425087f, 0.003867009188979864f, 0.018061378970742226f, 0.0063301147893071175f, 0.010813494212925434f, 0.006799081340432167f, 0.0058449069038033485f, 0.016704853624105453f, 0.008764450438320637f, 0.015856707468628883f, 0.027588073164224625f, 0.003872357774525881f, 0.02820579521358013f, 0.013833919540047646f, 0.009241212159395218f, 0.025422366335988045f, 0.012411439791321754f, 0.008885473944246769f, 0.003982515539973974f, 0.05299227684736252f, 0.010584310628473759f, 0.004182084463536739f, 0.01438275445252657f, 0.009455054998397827f, 0.004183530807495117f, 0.019208570942282677f, 0.03428531065583229f, 0.024704579263925552f, 0.002590765245258808f, 0.016996948048472404f, 0.009737804532051086f, 0.008195209316909313f, 0.009036922827363014f, 0.019314046949148178f, 0.006261386442929506f, 0.010925410315394402f, 0.012156162410974503f, 0.011942865327000618f, 0.011539324186742306f, 0.011895911768078804f, 0.022003404796123505f, 0.011227495968341827f, 0.008862958289682865f, 0.004033822566270828f, 0.008933992125093937f, 0.020893102511763573f, 0.02293243445456028f, 0.009386977180838585f, 0.019564690068364143f, 0.010089614428579807f, 0.01593506708741188f, 0.011338754557073116f, 0.014757497236132622f, 0.014371352270245552f, 0.005266158375889063f, 0.05017509683966637f, 0.028637494891881943f, 0.01362529955804348f, 0.018815910443663597f, 0.005041300784796476f);
static const ai_u16 conv2d_5_t_out_0_shape_w_const_u16 = 16;
static const ai_u16 conv2d_5_t_out_0_shape_h_const_u16 = 16;

static const ai_u16 conv2d_6_t_in_0_shape_w_const_u16 = 16;
static const ai_u16 conv2d_6_t_in_0_shape_h_const_u16 = 16;
static const ai_u16 conv2d_6_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_6_l_stride_0_const_u16 = 1;
static const ai_u16 conv2d_6_t_in_0_shape_ch_const_u16 = 128;
static const ai_u16 conv2d_6_t_out_0_shape_ch_const_u16 = 256;
static const ai_i8 conv2d_6_t_in_0_fmt_zero_const_s8 = 2;
static const ai_i8 conv2d_6_t_out_0_fmt_zero_const_s8 = -128;
static const ai_float conv2d_6_t_in_0_fmt_scale_const_f32 = 0.1389806717634201f;
static const ai_float conv2d_6_t_out_0_fmt_scale_const_f32 = 0.08044284582138062f;
static const ai_float conv2d_6_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.003772847354412079f, 0.0011713317362591624f, 0.0018648251425474882f, 0.0021558485459536314f, 0.0013750033685937524f, 0.0017044588457792997f, 0.0030188646633177996f, 0.0015816043596714735f, 0.0027431382331997156f, 0.0015615577576681972f, 0.0028846552595496178f, 0.0010971124283969402f, 0.002882012166082859f, 0.004213436506688595f, 0.0009581471676938236f, 0.0007970319129526615f, 0.0012237917399033904f, 0.002123893704265356f, 0.0013813241384923458f, 0.0025739218108356f, 0.000832719262689352f, 0.001043859519995749f, 0.0012110203970223665f, 0.0012837664689868689f, 0.0033779852092266083f, 0.0026682133320719004f, 0.002316970843821764f, 0.0031102129723876715f, 0.0019410899840295315f, 0.0029555181972682476f, 0.0015001301653683186f, 0.0018269370775669813f, 0.0024201548658311367f, 0.0012152447598055005f, 0.0008854675106704235f, 0.0013293367810547352f, 0.0019108435371890664f, 0.002115816343575716f, 0.0017996985698118806f, 0.0033792383037507534f, 0.0028138081543147564f, 0.0027617488522082567f, 0.0012004980817437172f, 0.0014405985129997134f, 0.0032103073317557573f, 0.005830311682075262f, 0.0015847895992919803f, 0.0014377088518813252f, 0.004727971740067005f, 0.0018742174142971635f, 0.0018731558229774237f, 0.0009905877523124218f, 0.005180795677006245f, 0.0019028661772608757f, 0.0005014521302655339f, 0.00146618508733809f, 0.0023094124626368284f, 0.000546834955457598f, 0.0010870762635022402f, 0.0016311627114191651f, 0.002520205918699503f, 0.001343001495115459f, 0.0036818517837673426f, 0.002413770416751504f, 0.0018157016020268202f, 0.0022189945448189974f, 0.0022970291320234537f, 0.0008447834989055991f, 0.0013077938929200172f, 0.0013207289157435298f, 0.002051860559731722f, 0.0010942535009235144f, 0.002341731684282422f, 0.0014532977947965264f, 0.001480261911638081f, 0.0012320755049586296f, 0.0014069689204916358f, 0.004097354598343372f, 0.0019626652356237173f, 0.0016995823243632913f, 0.0011926358565688133f, 0.0021735168993473053f, 0.002224687021225691f, 0.0012176340678706765f, 0.0014726606896147132f, 0.0019925786182284355f, 0.003910040948539972f, 0.0013082883087918162f, 0.0015008477494120598f, 0.0006128188106231391f, 0.0007825242937542498f, 0.0020087636075913906f, 0.0015378014650195837f, 0.00178536307066679f, 0.0030711814761161804f, 0.006041551008820534f, 0.0012398854596540332f, 0.0024773995392024517f, 0.0011303825303912163f, 0.0015289878938347101f, 0.0011621987214311957f, 0.001595867215655744f, 0.0029456738848239183f, 0.004593281075358391f, 0.003183661727234721f, 0.0005791780422441661f, 0.002009779680520296f, 0.0035813141148537397f, 0.0022963578812777996f, 0.00274404208175838f, 0.0011322820791974664f, 0.005045227240771055f, 0.0015895337564870715f, 0.001378283486701548f, 0.0008594974642619491f, 0.0016773612005636096f, 0.0025437050499022007f, 0.0014701756881549954f, 0.0007058928604237735f, 0.0015166578814387321f, 0.0007123418617993593f, 0.003010950516909361f, 0.0011329621775075793f, 0.0012708811555057764f, 0.0041497875936329365f, 0.00127177604008466f, 0.0012036011321470141f, 0.0018192764837294817f, 0.001496017794124782f, 0.0021273395977914333f, 0.002729448489844799f, 0.0013985905097797513f, 0.0015002274885773659f, 0.0009628614643588662f, 0.00190843571908772f, 0.0025827675126492977f, 0.0020600254647433758f, 0.0019731088541448116f, 0.0020531045738607645f, 0.0023384944070130587f, 0.0031704704742878675f, 0.0016243106219917536f, 0.0015226800460368395f, 0.0013619719538837671f, 0.002215464599430561f, 0.0028485464863479137f, 0.003962613642215729f, 0.0017550112679600716f, 0.0034142574295401573f, 0.00451292609795928f, 0.0017561048734933138f, 0.002774705644696951f, 0.006000580731779337f, 0.0006772699998691678f, 0.002989829983562231f, 0.0018656071042641997f, 0.0018447189358994365f, 0.002039549173787236f, 0.0010973470052704215f, 0.0024155271239578724f, 0.0019761151634156704f, 0.0013319548452273011f, 0.004081894177943468f, 0.0017050582682713866f, 0.0034018591977655888f, 0.0014232904650270939f, 0.0017421444645151496f, 0.0019154245965182781f, 0.001163666369393468f, 0.0018384844297543168f, 0.0018818594980984926f, 0.003223457606509328f, 0.0008521022391505539f, 0.0033384915441274643f, 0.0009512959513813257f, 0.002937222132459283f, 0.0028440675232559443f, 0.0028609102591872215f, 0.0030704394448548555f, 0.0011499273823574185f, 0.005961056798696518f, 0.0017473323969170451f, 0.001651994651183486f, 0.0020103221759200096f, 0.0014657477149739861f, 0.001197574776597321f, 0.0036427495069801807f, 0.0018570488318800926f, 0.0022253794595599174f, 0.0036372768227010965f, 0.002338992664590478f, 0.0019074955489486456f, 0.0007154660997912288f, 0.0016227707965299487f, 0.001119296532124281f, 0.01017654687166214f, 0.0012070740340277553f, 0.0016101360088214278f, 0.001134479301981628f, 0.0012814898509532213f, 0.0022853869013488293f, 0.0031833983957767487f, 0.0014693342382088304f, 0.00244967476464808f, 0.0022371765226125717f, 0.0026424566749483347f, 0.001634521409869194f, 0.0032383485231548548f, 0.0014755037846043706f, 0.0027227699756622314f, 0.0016619075322523713f, 0.0021926024928689003f, 0.0012269625440239906f, 0.003281471086665988f, 0.0016488913679495454f, 0.0016817825380712748f, 0.002085228683426976f, 0.0028932702261954546f, 0.002730043837800622f, 0.005902842152863741f, 0.0024068064521998167f, 0.0009310568566434085f, 0.0029468555003404617f, 0.0028244424611330032f, 0.0017853230237960815f, 0.000668547407258302f, 0.0016585933044552803f, 0.00214708736166358f, 0.002040317514911294f, 0.0030281224753707647f, 0.002441135700792074f, 0.0016447807429358363f, 0.0014181112637743354f, 0.0013902724022045732f, 0.004826079122722149f, 0.002664932981133461f, 0.002762971678748727f, 0.00437469407916069f, 0.003470450406894088f, 0.00231459503993392f, 0.0014659555163234472f, 0.004432107787579298f, 0.00157506566029042f, 0.0016597427893429995f, 0.0027892333455383778f, 0.0033263061195611954f, 0.0021802238188683987f, 0.0011993465013802052f, 0.0015385791193693876f, 0.0013980212388560176f, 0.001417841762304306f, 0.00105736730620265f, 0.0014750231057405472f, 0.002055132994428277f, 0.0014437271747738123f, 0.0015343388076871634f);
static const ai_layer_format_type conv2d_6_l_out_ch_format_const_layer_format_type = AI_LAYER_FORMAT_CHANNEL_FIRST_SAME;


static const ai_u16 conv2d_7_t_in_0_shape_w_const_u16 = 18;
static const ai_u16 conv2d_7_t_in_0_shape_h_const_u16 = 18;
static const ai_u16 conv2d_7_t_in_0_shape_ch_const_u16 = 256;
static const ai_u16 conv2d_7_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_7_l_stride_0_const_u16 = 1;
static const ai_i8 conv2d_7_t_in_0_fmt_zero_const_s8 = -128;
static const ai_i8 conv2d_7_t_out_0_fmt_zero_const_s8 = 2;
static const ai_float conv2d_7_t_in_0_fmt_scale_const_f32 = 0.08044284582138062f;
static const ai_float conv2d_7_t_out_0_fmt_scale_const_f32 = 0.2299330234527588f;
static const ai_float conv2d_7_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.00687447702512145f, 0.01573885791003704f, 0.006849820259958506f, 0.015601305291056633f, 0.017422828823328018f, 0.008258718065917492f, 0.007207685150206089f, 0.008236111141741276f, 0.0057890345342457294f, 0.010594521649181843f, 0.006407083943486214f, 0.022744443267583847f, 0.015373250469565392f, 0.004896718077361584f, 0.008903403766453266f, 0.04085381701588631f, 0.024161266162991524f, 0.005801680497825146f, 0.008284158073365688f, 0.008115824311971664f, 0.018017467111349106f, 0.015177191235125065f, 0.01044212281703949f, 0.009291460737586021f, 0.007711721584200859f, 0.005525154992938042f, 0.005407621618360281f, 0.003958070185035467f, 0.007861731573939323f, 0.004907443653792143f, 0.008887712843716145f, 0.006303707603365183f, 0.005171619355678558f, 0.010843582451343536f, 0.02121775783598423f, 0.015433907508850098f, 0.009430457837879658f, 0.01090173702687025f, 0.005074826069176197f, 0.018977826461195946f, 0.003984985873103142f, 0.006656473968178034f, 0.014589816331863403f, 0.011667238548398018f, 0.005054121371358633f, 0.003909641411155462f, 0.013411922380328178f, 0.011701448820531368f, 0.005820595193654299f, 0.017713403329253197f, 0.011033977381885052f, 0.022343982011079788f, 0.003933031111955643f, 0.006038419436663389f, 0.014378678984940052f, 0.012449558824300766f, 0.004599542822688818f, 0.033604465425014496f, 0.022309688851237297f, 0.010020086541771889f, 0.009561948478221893f, 0.011141196824610233f, 0.0054743182845413685f, 0.012721538543701172f, 0.013872862793505192f, 0.006363559979945421f, 0.006082480773329735f, 0.0292508602142334f, 0.01821869984269142f, 0.011209500022232533f, 0.011390751227736473f, 0.007943585515022278f, 0.01335118804126978f, 0.015403208322823048f, 0.01571611687541008f, 0.022127024829387665f, 0.01349677611142397f, 0.005809991620481014f, 0.005738262087106705f, 0.012136423029005527f, 0.00934874638915062f, 0.008534027263522148f, 0.00919841043651104f, 0.01194859854876995f, 0.007922298274934292f, 0.012077217921614647f, 0.01483594998717308f, 0.010675163939595222f, 0.01065504178404808f, 0.021526403725147247f, 0.02622879296541214f, 0.011953599750995636f, 0.016484463587403297f, 0.0064383139833807945f, 0.004345287103205919f, 0.0018253817688673735f, 0.006110779009759426f, 0.008488178253173828f, 0.018326442688703537f, 0.01130968239158392f, 0.017389606684446335f, 0.010067857801914215f, 0.0042883604764938354f, 0.005264926236122847f, 0.00982877891510725f, 0.029577108100056648f, 0.007918531075119972f, 0.005775232799351215f, 0.010331617668271065f, 0.010695370845496655f, 0.012746328487992287f, 0.017281435430049896f, 0.008111994713544846f, 0.013084929436445236f, 0.015862328931689262f, 0.0099874222651124f, 0.018034664914011955f, 0.007638690993189812f, 0.028078805655241013f, 0.009749879129230976f, 0.04080551490187645f, 0.004194664303213358f, 0.014183013699948788f, 0.011366108432412148f, 0.004703742917627096f, 0.03318415954709053f, 0.016286971047520638f, 0.008739677257835865f, 0.009071726351976395f, 0.01534706074744463f, 0.004138358402997255f, 0.010425831191241741f, 0.012506593018770218f, 0.14567895233631134f, 0.01516028307378292f, 0.0073780640959739685f, 0.011171987280249596f, 0.006272167433053255f, 0.021875742822885513f, 0.009869460016489029f, 0.005168976727873087f, 0.0063628568314015865f, 0.010956433601677418f, 0.01232336275279522f, 0.005947438068687916f, 0.0034531215205788612f, 0.005547358654439449f, 0.012466918677091599f, 0.006077089346945286f, 0.003927413374185562f, 0.007033654488623142f, 0.004823729861527681f, 0.002926518674939871f, 0.02660582773387432f, 0.007055134512484074f, 0.0072896829806268215f, 0.0068676709197461605f, 0.010765484534204006f, 0.01188999880105257f, 0.006305342074483633f, 0.004648942034691572f, 0.006809841375797987f, 0.004930111113935709f, 0.012620797380805016f, 0.009218788705766201f, 0.010359557345509529f, 0.01785299740731716f, 0.013147521764039993f, 0.060830287635326385f, 0.011083447374403477f, 0.009799263440072536f, 0.005903307814151049f, 0.283989280462265f, 0.0048525468446314335f, 0.010819477029144764f, 0.005770692601799965f, 0.010874379426240921f, 0.00685805594548583f, 0.005987308919429779f, 0.0198451466858387f, 0.0025534818414598703f, 0.00665023410692811f, 0.009839045815169811f, 0.008455941453576088f, 0.014230155386030674f, 0.011695034801959991f, 0.00550411781296134f, 0.009180493652820587f, 0.008248351514339447f, 0.005897854920476675f, 0.007129139266908169f, 0.013425241224467754f, 0.016301827505230904f, 0.005742514040321112f, 0.00916194450110197f, 0.002925591543316841f, 0.01609628088772297f, 0.015864307060837746f, 0.028800275176763535f, 0.033271245658397675f, 0.007098935078829527f, 0.004638885613530874f, 0.017668340355157852f, 0.010567390359938145f, 0.009148890152573586f, 0.006875640247017145f, 0.017996229231357574f, 0.0037252819165587425f, 0.03709408640861511f, 0.004636405035853386f, 0.006506140343844891f, 0.01069517433643341f, 0.009405438788235188f, 0.0090252086520195f, 0.010990463197231293f, 0.008772233501076698f, 0.009151182137429714f, 0.0071806819178164005f, 0.007616003975272179f, 0.003014888847246766f, 0.006892993580549955f, 0.014289237558841705f, 0.005402269307523966f, 0.004599406383931637f, 0.00814387109130621f, 0.013141516596078873f, 0.011590859852731228f, 0.009115868248045444f, 0.007089396473020315f, 0.006852041929960251f, 0.005690020974725485f, 0.015579489059746265f, 0.009091483429074287f, 0.010493061505258083f, 0.007761679589748383f, 0.004836170934140682f, 0.010074488818645477f, 0.0046036639250814915f, 0.004955274984240532f, 0.0074211773462593555f, 0.018127430230379105f, 0.007993745617568493f, 0.008983046747744083f, 0.014202064834535122f, 0.007532574236392975f, 0.006258753128349781f, 0.009931695647537708f, 0.013750306330621243f, 0.057210955768823624f, 0.020376531407237053f, 0.008310282602906227f, 0.010836220346391201f, 0.01600847765803337f, 0.01238993275910616f, 0.01600048504769802f, 0.011271608993411064f);
static const ai_u16 conv2d_7_t_out_0_shape_w_const_u16 = 16;
static const ai_u16 conv2d_7_t_out_0_shape_h_const_u16 = 16;

static const ai_u16 conv2d_8_t_in_0_shape_w_const_u16 = 16;
static const ai_u16 conv2d_8_t_in_0_shape_h_const_u16 = 16;
static const ai_u16 conv2d_8_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_8_l_stride_0_const_u16 = 1;
static const ai_u16 conv2d_8_t_in_0_shape_ch_const_u16 = 256;
static const ai_u16 conv2d_8_t_out_0_shape_ch_const_u16 = 256;
static const ai_i8 conv2d_8_t_in_0_fmt_zero_const_s8 = 2;
static const ai_i8 conv2d_8_t_out_0_fmt_zero_const_s8 = -128;
static const ai_float conv2d_8_t_in_0_fmt_scale_const_f32 = 0.2299330234527588f;
static const ai_float conv2d_8_t_out_0_fmt_scale_const_f32 = 0.05152946338057518f;
static const ai_float conv2d_8_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.0006293772021308541f, 0.0007976819179020822f, 0.0013231897028163075f, 0.0013226376613602042f, 0.0006980136386118829f, 0.0011224595364183187f, 0.001126413349993527f, 0.0009927735663950443f, 0.0008833049214445055f, 0.000920471444260329f, 0.001579430652782321f, 0.0007643782300874591f, 0.0007957764319144189f, 0.000985182006843388f, 0.0006980430334806442f, 0.00171262479852885f, 0.0010120585793629289f, 0.0006518104928545654f, 0.0014458743389695883f, 0.0011176414554938674f, 0.001487987581640482f, 0.0009776350343599916f, 0.0009110877872444689f, 0.0013859565369784832f, 0.0011245718924328685f, 0.0014513528440147638f, 0.0013029637048020959f, 0.0011501330882310867f, 0.0008560820133425295f, 0.0007859990582801402f, 0.0016271290369331837f, 0.0017966007580980659f, 0.0015701091615483165f, 0.0006097768200561404f, 0.0009397296817041934f, 0.001041421084664762f, 0.00061592529527843f, 0.0008009059238247573f, 0.0011619923170655966f, 0.0019089265260845423f, 0.001126746879890561f, 0.002206025877967477f, 0.0009396171080879867f, 0.000618207675870508f, 0.0028939642943441868f, 0.0007816613651812077f, 0.0006078723235987127f, 0.002448936691507697f, 0.001139350002631545f, 0.0007667354075238109f, 0.0020697335712611675f, 0.0007520519429817796f, 0.002636274788528681f, 0.0007750882068648934f, 0.0007345993653871119f, 0.0008629771764390171f, 0.0007846861844882369f, 0.0005404695402830839f, 0.0010964861139655113f, 0.00018919713329523802f, 0.0010746705811470747f, 0.0014112117933109403f, 0.001047666184604168f, 0.0009761430555954576f, 0.0010898784967139363f, 0.0012752818875014782f, 0.0006478182622231543f, 0.0013491378631442785f, 0.0008948471513576806f, 0.0015088992659002542f, 0.001267333049327135f, 0.0016948838019743562f, 0.0009198824991472065f, 0.0008382467785850167f, 0.0015449877828359604f, 0.0014285146025940776f, 0.0005004281993024051f, 0.0005946404999122024f, 0.001750459079630673f, 0.0009306874708272517f, 0.001362790702842176f, 0.001051066443324089f, 0.0009420289425179362f, 0.001438475912436843f, 0.0004346626519691199f, 0.001206692075356841f, 0.000627007451839745f, 0.002183889038860798f, 0.0007777499849908054f, 0.0006265562260523438f, 0.002127598272636533f, 0.001016016467474401f, 0.0008223899640142918f, 0.0017479030648246408f, 0.0012478684075176716f, 0.0006509270169772208f, 0.0007368706865236163f, 0.0011099916882812977f, 0.0021229616831988096f, 0.0006865014438517392f, 0.0011205531191080809f, 0.0015173641731962562f, 0.0007426423253491521f, 0.0007367661455646157f, 0.002436369191855192f, 0.0009046410559676588f, 0.00041078252252191305f, 0.001725150621496141f, 0.0016829263186082244f, 0.001283960067667067f, 0.0012567956000566483f, 0.000806373602245003f, 0.0008871500031091273f, 0.0008600320434197783f, 0.0008151429938152432f, 0.0030078415293246508f, 0.001011007814668119f, 0.0007371242973022163f, 0.0026611716020852327f, 0.0011402040254324675f, 0.0008078849641606212f, 0.0011314351577311754f, 0.0017402912490069866f, 0.0005128254415467381f, 0.0010372309479862452f, 0.0006227025878615677f, 0.000918213336262852f, 0.0016970839351415634f, 0.0007520355866290629f, 0.0015042523154988885f, 0.0013861411716789007f, 0.001914768130518496f, 0.0012606758391484618f, 0.0019020582549273968f, 0.0020547392778098583f, 0.0013842411572113633f, 0.0009352471097372472f, 0.0008573758532293141f, 0.0017617428675293922f, 0.00081815873272717f, 0.0013101226650178432f, 0.0008328626281581819f, 0.00144989846739918f, 0.0006543690105900168f, 0.0013431995175778866f, 0.0008618638967163861f, 0.0030821748077869415f, 0.0010471467394381762f, 0.0007237432873807847f, 0.0019342582672834396f, 0.0016807514475658536f, 0.001030186889693141f, 0.0012267724378034472f, 0.0006916082347743213f, 0.0008186358027160168f, 0.0005073708016425371f, 0.0006672898307442665f, 0.0008202460594475269f, 0.0012224051170051098f, 0.001166649512015283f, 0.0016656400403007865f, 0.0007724484312348068f, 0.0009475472616031766f, 0.0006252456223592162f, 0.0020533488132059574f, 0.0009093450498767197f, 0.0006190395797602832f, 0.0007052978617139161f, 0.0021858634427189827f, 0.0013447124511003494f, 0.0010860388865694404f, 0.001130429096519947f, 0.00143466517329216f, 0.0009819885017350316f, 0.0014213405083864927f, 0.001038966584019363f, 0.0062003107741475105f, 0.0006813927320763469f, 0.0009791833581402898f, 0.000694542657583952f, 0.0014306403463706374f, 0.0008180496515706182f, 0.001676961430348456f, 0.0005807146662846208f, 0.001448270631954074f, 0.0006802676361985505f, 0.0017399413045495749f, 0.0006740788230672479f, 0.0005737338797189295f, 0.0008563111769035459f, 0.0029507705476135015f, 0.0007816688157618046f, 0.004729173146188259f, 0.0008265902288258076f, 0.0015677530318498611f, 0.0031177678611129522f, 0.0014800481731072068f, 0.0006265502306632698f, 0.0016485416563227773f, 0.0017942852573469281f, 0.000775302411057055f, 0.0011621970916166902f, 0.0045125409960746765f, 0.0006690123700536788f, 0.0010276335524395108f, 0.0012510899687185884f, 0.002688682870939374f, 0.0012626356910914183f, 0.0010147513821721077f, 0.0010123823303729296f, 0.0009938335279002786f, 0.0008072067867033184f, 0.0010604967828840017f, 0.0023884770926088095f, 0.0012950485106557608f, 0.0008528687758371234f, 0.0008176825358532369f, 0.0009463970782235265f, 0.0016108826966956258f, 0.001277851639315486f, 0.0019234513165429235f, 0.001535030547529459f, 0.0006972602568566799f, 0.0010378859005868435f, 0.0008519967086613178f, 0.0007641052943654358f, 0.002435040194541216f, 0.0008392283343710005f, 0.0019297098042443395f, 0.0007476932369172573f, 0.0008486370206810534f, 0.0010827729711309075f, 0.0010582224931567907f, 0.0010684559820219874f, 0.0018428716575726867f, 0.0009674143511801958f, 0.002395703922957182f, 0.002155957743525505f, 0.0006297632935456932f, 0.0008116880781017244f, 0.0015762849943712354f, 0.0026789591647684574f, 0.0013519166968762875f, 0.0011907556327059865f, 0.0011850502341985703f, 0.001147474511526525f, 0.0014207289787009358f, 0.0011529689654707909f, 0.000798154273070395f, 0.0010925951646640897f, 0.0010651065967977047f, 0.0017818582709878683f, 0.0017764706863090396f, 0.0013909952249377966f, 0.001604521763511002f, 0.00019862047338392586f);
static const ai_layer_format_type conv2d_8_l_out_ch_format_const_layer_format_type = AI_LAYER_FORMAT_CHANNEL_LAST_VALID;

static const ai_u16 pool_9_t_in_0_shape_w_const_u16 = 16;
static const ai_u16 pool_9_t_in_0_shape_h_const_u16 = 16;
static const ai_u16 pool_9_t_in_0_shape_ch_const_u16 = 256;
static const ai_u16 pool_9_l_pool_size_1_const_u16 = 2;
static const ai_u16 pool_9_l_pool_size_0_const_u16 = 2;
static const ai_u16 pool_9_l_legacy_pool_pad_1_const_u16 = 0;
static const ai_u16 pool_9_l_legacy_pool_pad_0_const_u16 = 0;
static const ai_u16 pool_9_l_pool_stride_1_const_u16 = 1;
static const ai_u16 pool_9_l_pool_stride_0_const_u16 = 1;
static const ai_u16 pool_9_t_out_0_shape_w_const_u16 = 16;
static const ai_u16 pool_9_t_out_0_shape_h_const_u16 = 16;
static const ai_i8 pool_9_t_in_0_fmt_zero_const_s8 = -128;
static const ai_i8 pool_9_t_out_0_fmt_zero_const_s8 = -128;

static const ai_i8 conv2d_10_pad_before_v_pad_constant_value_const_s8[] = LITE_ARRAY_VALUES(-128);
static const ai_i16 conv2d_10_pad_before_t_in_0_fmt_bitsize_const_s16 = 8;
static const ai_u32 conv2d_10_pad_before_t_in_0_shape_h_const_u32 = 16;

static const ai_u16 conv2d_10_t_in_0_shape_w_const_u16 = 18;
static const ai_u16 conv2d_10_t_in_0_shape_h_const_u16 = 18;
static const ai_u16 conv2d_10_t_in_0_shape_ch_const_u16 = 256;
static const ai_u16 conv2d_10_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_10_l_stride_0_const_u16 = 1;
static const ai_i8 conv2d_10_t_in_0_fmt_zero_const_s8 = -128;
static const ai_i8 conv2d_10_t_out_0_fmt_zero_const_s8 = 20;
static const ai_float conv2d_10_t_in_0_fmt_scale_const_f32 = 0.05152946338057518f;
static const ai_float conv2d_10_t_out_0_fmt_scale_const_f32 = 0.1385585218667984f;
static const ai_float conv2d_10_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.023123683407902718f, 0.007484226953238249f, 0.0066726733930408955f, 0.027892637997865677f, 0.036648429930210114f, 0.050324540585279465f, 0.009990434162318707f, 0.023984745144844055f, 0.0068678404204547405f, 0.013763956725597382f, 0.011585142463445663f, 0.047652941197156906f, 0.025340775027871132f, 0.02573498897254467f, 0.055379096418619156f, 0.008411393500864506f, 0.012552044354379177f, 0.009950593113899231f, 0.007973297499120235f, 0.018223389983177185f, 0.00860568042844534f, 0.017812194302678108f, 0.011672552675008774f, 0.016673246398568153f, 0.01307616475969553f, 0.003739903448149562f, 0.006806242745369673f, 0.011100957170128822f, 0.008172816596925259f, 0.01919170655310154f, 0.0105615658685565f, 0.013532246463000774f, 0.01145891658961773f, 0.022712238132953644f, 0.013060747645795345f, 0.015150750055909157f, 0.02750004455447197f, 0.025246359407901764f, 0.014932538382709026f, 0.004967768210917711f, 0.0177316851913929f, 0.003547737840563059f, 0.008936312980949879f, 0.012795805931091309f, 0.018343495205044746f, 0.025835227221250534f, 0.013597930781543255f, 0.0037543517537415028f, 0.005786886904388666f, 0.015432305634021759f, 0.003925117198377848f, 0.007378153502941132f, 0.004220221657305956f, 0.0227485541254282f, 0.02013300731778145f, 0.04109394922852516f, 0.024938175454735756f, 0.05645836144685745f, 0.011044267565011978f, 0.19554023444652557f, 0.012171013280749321f, 0.014547222293913364f, 0.02143191546201706f, 0.01083004754036665f, 0.011785265989601612f, 0.005896867252886295f, 0.03826089948415756f, 0.009638085961341858f, 0.05625762417912483f, 0.00733671011403203f, 0.007133769802749157f, 0.018266059458255768f, 0.009068770334124565f, 0.014627211727201939f, 0.018155774101614952f, 0.00633349921554327f, 0.034231629222631454f, 0.023667383939027786f, 0.013010717928409576f, 0.056467268615961075f, 0.018800891935825348f, 0.009717409498989582f, 0.008369788527488708f, 0.005956798326224089f, 0.05548067390918732f, 0.018727710470557213f, 0.031376466155052185f, 0.0062888264656066895f, 0.03536536917090416f, 0.01356157474219799f, 0.00674178171902895f, 0.022552018985152245f, 0.017119305208325386f, 0.01420386228710413f, 0.007901322096586227f, 0.02451835758984089f, 0.016060901805758476f, 0.007778299041092396f, 0.002737639704719186f, 0.03550265356898308f, 0.01939547248184681f, 0.008472602814435959f, 0.01647544838488102f, 0.017705386504530907f, 0.017804041504859924f, 0.00814817938953638f, 0.08383548259735107f, 0.009954193606972694f, 0.011726138181984425f, 0.010336583480238914f, 0.01264291349798441f, 0.019677748903632164f, 0.01075985748320818f, 0.010832641273736954f, 0.0226272065192461f, 0.011817602440714836f, 0.022038983181118965f, 0.03590305149555206f, 0.002800202462822199f, 0.03325745090842247f, 0.007483656983822584f, 0.009755712933838367f, 0.004941754974424839f, 0.04777102172374725f, 0.029189303517341614f, 0.02510441653430462f, 0.01049518957734108f, 0.015155784785747528f, 0.009380408562719822f, 0.00910173449665308f, 0.008225319907069206f, 0.006233261898159981f, 0.011499314568936825f, 0.010172219946980476f, 0.10549495369195938f, 0.015636418014764786f, 0.017660319805145264f, 0.013404910452663898f, 0.013788547366857529f, 0.011777268722653389f, 0.007620963267982006f, 0.01588395982980728f, 0.017419060692191124f, 0.021523084491491318f, 0.008354300633072853f, 0.013169052079319954f, 0.0015646446263417602f, 0.007762467954307795f, 0.016323938965797424f, 0.0081104701384902f, 0.002880231011658907f, 0.012778392992913723f, 0.034154560416936874f, 0.014377981424331665f, 0.02036723680794239f, 0.012839968316257f, 0.027434376999735832f, 0.019367914646863937f, 0.0076628257520496845f, 0.0033486306201666594f, 0.002902367152273655f, 0.019869064912199974f, 0.011580927297472954f, 0.015945248305797577f, 0.01201232336461544f, 0.015682807192206383f, 0.04867476969957352f, 0.00912702176719904f, 0.008612719364464283f, 0.006782355718314648f, 0.026322046294808388f, 0.015851283445954323f, 0.008202449418604374f, 0.0081911850720644f, 0.020017236471176147f, 0.017444472759962082f, 0.011832544580101967f, 0.00845083687454462f, 0.010579351335763931f, 0.025808721780776978f, 0.011378374882042408f, 0.02008225955069065f, 0.009627796709537506f, 0.014692531898617744f, 0.006269174627959728f, 0.015363897196948528f, 0.018677111715078354f, 0.06800039112567902f, 0.030781641602516174f, 0.027097824960947037f, 0.003885838435962796f, 0.011422023177146912f, 0.0055714440532028675f, 0.014766540378332138f, 0.00773600721731782f, 0.002242082031443715f, 0.015130452811717987f, 0.030568376183509827f, 0.00838658306747675f, 0.013952328823506832f, 0.02254069596529007f, 0.01224488951265812f, 0.004454239271581173f, 0.016740359365940094f, 0.009609386324882507f, 0.01010515633970499f, 0.0043149362318217754f, 0.006336253602057695f, 0.008447499945759773f, 0.016454951837658882f, 0.0190858356654644f, 0.01657751388847828f, 0.013418520800769329f, 0.00690904026851058f, 0.013309682719409466f, 0.014636953361332417f, 0.005412201397120953f, 0.01710532419383526f, 0.0026087320875376463f, 0.022283118218183517f, 0.0037858502473682165f, 0.005566946696490049f, 0.013150007463991642f, 0.04133637994527817f, 0.02890303172171116f, 0.031611714512109756f, 0.004421232733875513f, 0.01892464980483055f, 0.008913630619645119f, 0.009528241120278835f, 0.01241390872746706f, 0.00997447781264782f, 0.009848055429756641f, 0.05074893683195114f, 0.01748954877257347f, 0.008086113259196281f, 0.0029714978300035f, 0.005901303607970476f, 0.01727404072880745f, 0.013418087735772133f, 0.005954309832304716f, 0.005209488794207573f, 0.014803064987063408f, 0.009232808835804462f, 0.010188449174165726f, 0.01876657083630562f, 0.011131447739899158f, 0.009360389783978462f, 0.04850463569164276f, 0.012545652687549591f, 0.01424633152782917f, 0.009335067123174667f, 0.023228852078318596f, 0.007377708796411753f, 0.00786927342414856f, 0.033990468829870224f);
static const ai_u16 conv2d_10_t_out_0_shape_w_const_u16 = 16;
static const ai_u16 conv2d_10_t_out_0_shape_h_const_u16 = 16;

static const ai_u16 conv2d_11_t_in_0_shape_w_const_u16 = 16;
static const ai_u16 conv2d_11_t_in_0_shape_h_const_u16 = 16;
static const ai_u16 conv2d_11_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_11_l_stride_0_const_u16 = 1;
static const ai_u16 conv2d_11_t_in_0_shape_ch_const_u16 = 256;
static const ai_u16 conv2d_11_t_out_0_shape_ch_const_u16 = 128;
static const ai_i8 conv2d_11_t_in_0_fmt_zero_const_s8 = 20;
static const ai_i8 conv2d_11_t_out_0_fmt_zero_const_s8 = -128;
static const ai_float conv2d_11_t_in_0_fmt_scale_const_f32 = 0.1385585218667984f;
static const ai_float conv2d_11_t_out_0_fmt_scale_const_f32 = 0.10237286239862442f;
static const ai_float conv2d_11_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.003936059772968292f, 0.0011378948111087084f, 0.0012206844985485077f, 0.0011468221200630069f, 0.001713404431939125f, 0.000962293881457299f, 0.0008231130777858198f, 0.0007592166075482965f, 0.0007339939475059509f, 0.001954068196937442f, 0.0030171512626111507f, 0.00037526822416111827f, 0.0005370313301682472f, 0.0032562583219259977f, 0.0012028461787849665f, 0.003250759094953537f, 0.013583843596279621f, 0.0013354531256482005f, 0.0015606600791215897f, 0.0029359334148466587f, 0.0013395085697993636f, 0.0014984240988269448f, 0.0007227276219055057f, 0.001137588289566338f, 0.0001805952051654458f, 0.0017076268559321761f, 0.0014914622297510505f, 0.0013217342784628272f, 0.0013207178562879562f, 0.000560747052077204f, 0.0016949360724538565f, 0.001330925733782351f, 0.001994018442928791f, 0.0014710555551573634f, 0.0006814661319367588f, 0.0005496907979249954f, 0.0010507508413866162f, 0.0007450615521520376f, 0.0019311794312670827f, 0.00048145352047868073f, 0.0005974748637527227f, 0.002957748482003808f, 0.005583104211837053f, 0.0036077473778277636f, 0.002629670314490795f, 0.007222553249448538f, 0.0011367565020918846f, 0.001071714679710567f, 0.0015483410097658634f, 0.0009123198105953634f, 0.002268752781674266f, 0.0008496591472066939f, 0.0016163644613698125f, 0.0003565099323168397f, 0.0017166559118777514f, 0.0013987385900691152f, 0.0016567722195759416f, 0.0018891718937084079f, 0.0009771534241735935f, 0.0009806513553485274f, 0.001187461894005537f, 0.0009741771500557661f, 0.004051086492836475f, 0.0001018540351651609f, 0.0007566348649561405f, 0.0014246746432036161f, 0.0017362291691824794f, 0.0013644257560372353f, 0.010810159146785736f, 0.0005916986265219748f, 0.001009801053442061f, 0.0010943925008177757f, 0.0011993705993518233f, 0.001477811485528946f, 0.00017851153097581118f, 0.003901076503098011f, 0.002768764505162835f, 0.0005422745016403496f, 0.0036294320598244667f, 0.0008983073639683425f, 0.0012676611077040434f, 0.0011400613002479076f, 0.00647698761895299f, 0.0006918812869116664f, 0.001045309123583138f, 0.0009811979252845049f, 0.0010854474967345595f, 0.0010233501670882106f, 0.0005358985508792102f, 0.0016664380673319101f, 0.0013459579786285758f, 0.0025381597224622965f, 0.00021847868629265577f, 0.004253201186656952f, 0.002796119311824441f, 0.0018827589228749275f, 0.0015704385004937649f, 0.0025071834679692984f, 0.0013620445970445871f, 0.0002674828574527055f, 0.0008809447172097862f, 0.00014114822261035442f, 0.000653848226647824f, 0.007439617067575455f, 0.002267449162900448f, 5.093015352031216e-05f, 0.001169340219348669f, 0.0011774784652516246f, 0.0007714048842899501f, 0.0007498024497181177f, 0.0017888258444145322f, 0.0010063692461699247f, 0.0016621508402749896f, 0.0005599178257398307f, 0.0008726100204512477f, 0.0001201269551529549f, 0.0007630186155438423f, 0.0030233461875468493f, 0.0009395885281264782f, 0.001524133956991136f, 0.0011574014788493514f, 0.0014961094129830599f, 0.0014102644054219127f, 9.947693615686148e-05f, 9.78972457232885e-05f, 0.0063906400464475155f, 0.0009334587957710028f, 0.001579268486239016f);
static const ai_layer_format_type conv2d_11_l_out_ch_format_const_layer_format_type = AI_LAYER_FORMAT_CHANNEL_FIRST_SAME;


static const ai_u16 conv2d_12_t_in_0_shape_w_const_u16 = 18;
static const ai_u16 conv2d_12_t_in_0_shape_h_const_u16 = 18;
static const ai_u16 conv2d_12_t_in_0_shape_ch_const_u16 = 128;
static const ai_u16 conv2d_12_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_12_l_stride_0_const_u16 = 1;
static const ai_i8 conv2d_12_t_in_0_fmt_zero_const_s8 = -128;
static const ai_i8 conv2d_12_t_out_0_fmt_zero_const_s8 = -28;
static const ai_float conv2d_12_t_in_0_fmt_scale_const_f32 = 0.10237286239862442f;
static const ai_float conv2d_12_t_out_0_fmt_scale_const_f32 = 0.13364781439304352f;
static const ai_float conv2d_12_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.0039006348233669996f, 0.007133289240300655f, 0.004648096393793821f, 0.014350653626024723f, 0.003515260061249137f, 0.00805718544870615f, 0.012349429540336132f, 0.02000817470252514f, 0.010922570712864399f, 0.00779730686917901f, 0.004185419995337725f, 0.055824752897024155f, 0.010243702679872513f, 0.001529452158138156f, 0.008257575333118439f, 0.008089627139270306f, 0.0010043152142316103f, 0.008326857350766659f, 0.005928063299506903f, 0.007369136903434992f, 0.004724291618913412f, 0.0030280312057584524f, 0.010584169998764992f, 0.011965411715209484f, 0.05890505388379097f, 0.003151669166982174f, 0.005878691095858812f, 0.0065932502038776875f, 0.01201468426734209f, 0.015046900138258934f, 0.004515279550105333f, 0.005297963507473469f, 0.004978081677109003f, 0.007018823642283678f, 0.017334280535578728f, 0.03980238735675812f, 0.0053641097620129585f, 0.00696070259436965f, 0.0059360540471971035f, 0.022530389949679375f, 0.018838487565517426f, 0.004366347566246986f, 0.0014019216177985072f, 0.0032231472432613373f, 0.0024646299425512552f, 0.0025244096759706736f, 0.004183129407465458f, 0.004441107623279095f, 0.005821303930133581f, 0.00873988214880228f, 0.002796830842271447f, 0.0217962097376585f, 0.0038362115155905485f, 0.11587370187044144f, 0.014771776273846626f, 0.008882755413651466f, 0.005369032267481089f, 0.004091107752174139f, 0.00797487422823906f, 0.01625981554389f, 0.005290470086038113f, 0.009573331102728844f, 0.001652575214393437f, 0.03509858250617981f, 0.009968921542167664f, 0.007992012426257133f, 0.004544545896351337f, 0.005857211537659168f, 0.001530730165541172f, 0.010147398337721825f, 0.007515404373407364f, 0.009073804132640362f, 0.004532108083367348f, 0.007808490190654993f, 0.035807378590106964f, 0.008297344669699669f, 0.003211801405996084f, 0.34272563457489014f, 0.0019489218248054385f, 0.010336336679756641f, 0.011776438914239407f, 0.007658296264708042f, 0.0016489096451550722f, 0.011763395741581917f, 0.010050245560705662f, 0.007395449560135603f, 0.004750629421323538f, 0.005663428921252489f, 0.008931484073400497f, 0.0056542144156992435f, 0.005969117861241102f, 0.005393671803176403f, 0.08449702709913254f, 0.0028364057652652264f, 0.003440181026235223f, 0.044331200420856476f, 0.004501246847212315f, 0.005513735115528107f, 0.014878006651997566f, 0.014953705482184887f, 0.012402316555380821f, 0.036641623824834824f, 0.00862361490726471f, 0.001420763903297484f, 0.004389623180031776f, 0.027414632961153984f, 0.0077493940480053425f, 0.00836265180259943f, 0.013173929415643215f, 0.009490340016782284f, 0.0043560960330069065f, 0.009792071767151356f, 0.008292101323604584f, 0.016313867643475533f, 0.015516065061092377f, 0.016690080985426903f, 0.010009916499257088f, 0.002936486154794693f, 0.019717101007699966f, 0.011065966449677944f, 0.010469499975442886f, 0.010100997984409332f, 0.005341155920177698f, 0.02989472635090351f, 0.00851855892688036f, 0.0032844599336385727f, 0.007210602052509785f, 0.0029731551185250282f);
static const ai_u16 conv2d_12_t_out_0_shape_w_const_u16 = 16;
static const ai_u16 conv2d_12_t_out_0_shape_h_const_u16 = 16;

static const ai_u16 conv2d_13_t_in_0_shape_w_const_u16 = 16;
static const ai_u16 conv2d_13_t_in_0_shape_h_const_u16 = 16;
static const ai_u16 conv2d_13_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_13_l_stride_0_const_u16 = 1;
static const ai_u16 conv2d_13_t_in_0_shape_ch_const_u16 = 128;
static const ai_u16 conv2d_13_t_out_0_shape_ch_const_u16 = 256;
static const ai_i8 conv2d_13_t_in_0_fmt_zero_const_s8 = -28;
static const ai_i8 conv2d_13_t_out_0_fmt_zero_const_s8 = -128;
static const ai_float conv2d_13_t_in_0_fmt_scale_const_f32 = 0.13364781439304352f;
static const ai_float conv2d_13_t_out_0_fmt_scale_const_f32 = 0.030627276748418808f;
static const ai_float conv2d_13_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.0033871300984174013f, 0.003019636496901512f, 0.002243174472823739f, 0.0019784497562795877f, 0.0015071913367137313f, 0.0017085501458495855f, 0.003070591716095805f, 0.003038285067304969f, 0.000991765409708023f, 0.001966855488717556f, 0.0017989089246839285f, 0.0018959949957206845f, 0.0012478544376790524f, 0.0064710467122495174f, 0.001757637714035809f, 0.0027197077870368958f, 0.0022792574018239975f, 0.0031971007119864225f, 0.0030002857092767954f, 0.0017081783153116703f, 0.004342361818999052f, 0.0028677708469331264f, 0.002661040285602212f, 0.0020482696127146482f, 0.001612018677406013f, 0.0040183281525969505f, 0.00201427866704762f, 0.00179388583637774f, 0.006239243783056736f, 0.0020902471151202917f, 0.001623807824216783f, 0.0021829004399478436f, 0.002590894466266036f, 0.002228345023468137f, 0.002308100927621126f, 0.0020174109376966953f, 0.0014626368647441268f, 0.002377850003540516f, 0.001966373296454549f, 0.00236181216314435f, 0.0019673844799399376f, 0.00427475618198514f, 0.0023699181620031595f, 0.0022125670220702887f, 0.0013705838937312365f, 0.002271234057843685f, 0.001344048883765936f, 0.002974666887894273f, 0.003090780461207032f, 0.0019185421988368034f, 0.0023857615888118744f, 0.002592639531940222f, 0.0015563670312985778f, 0.0033901387359946966f, 0.0021529707591980696f, 0.0019893061835318804f, 0.0023031358141452074f, 0.002183448988944292f, 0.0016373497201129794f, 0.0011310881236568093f, 0.0030214134603738785f, 0.003115792525932193f, 0.003147633047774434f, 0.0023180944845080376f, 0.0009075379348360002f, 0.002072715200483799f, 0.002217019209638238f, 0.003186509246006608f, 0.0019845208153128624f, 0.002425322774797678f, 0.0014858841896057129f, 0.0029547696467489004f, 0.0021419331897050142f, 0.0017750824335962534f, 0.002319423947483301f, 0.002924446016550064f, 0.0038602387066930532f, 0.0025085629895329475f, 0.0024103817995637655f, 0.0016781409503892064f, 0.0010418592719361186f, 0.0026500190142542124f, 0.0020530065521597862f, 0.005655552260577679f, 0.0036696328315883875f, 0.0024505965411663055f, 0.0023116685915738344f, 0.003904287703335285f, 0.001777107478119433f, 0.0017532361671328545f, 0.0021025105379521847f, 0.0030236137099564075f, 0.003711965400725603f, 0.0028822114691138268f, 0.0037183640524744987f, 0.0026909178122878075f, 0.002236994681879878f, 0.001693377154879272f, 0.0038151757325977087f, 0.00274074450135231f, 0.0015792333288118243f, 0.0027893714141100645f, 0.0030571408569812775f, 0.001006574253551662f, 0.0025420908350497484f, 0.0019249645993113518f, 0.0014509600587189198f, 0.0012965957866981626f, 0.0019612992182374f, 0.0026936065405607224f, 0.002406090497970581f, 0.0027773159090429544f, 0.0018930792575702071f, 0.0024033088702708483f, 0.001995264319702983f, 0.0037148697301745415f, 0.0019232165068387985f, 0.002635570243000984f, 0.0030307758133858442f, 0.0013922946527600288f, 0.0026280120946466923f, 0.0023210563231259584f, 0.0037236004136502743f, 0.0021321948152035475f, 0.0013210036559030414f, 0.0015525585040450096f, 0.0022401290480047464f, 0.002482609124854207f, 0.0017528083408251405f, 0.0016672855708748102f, 0.004076718352735043f, 0.002933489391580224f, 0.0021096880082041025f, 0.002060620579868555f, 0.002675156807526946f, 0.0012994775315746665f, 0.0019303184235468507f, 0.0019243537681177258f, 0.0027495096437633038f, 0.0022817354183644056f, 0.0025637478101998568f, 0.0018659719498828053f, 0.00344626372680068f, 0.0024364639539271593f, 0.001746642286889255f, 0.0047029489651322365f, 0.0022576951887458563f, 0.0014761689817532897f, 0.004744819365441799f, 0.0030382254626601934f, 0.002576818922534585f, 0.0019726722966879606f, 0.002479763003066182f, 0.0022517673205584288f, 0.003081650473177433f, 0.002870772033929825f, 0.0030163477640599012f, 0.002430172637104988f, 0.0020594371017068624f, 0.0034087325911968946f, 0.002477197675034404f, 0.0020309865940362215f, 0.0016976377228274941f, 0.0020508645102381706f, 0.0017443825490772724f, 0.0036731665022671223f, 0.0014534847578033805f, 0.0029977390076965094f, 0.0024793113116174936f, 0.0031345768366008997f, 0.002272768644616008f, 0.001690773875452578f, 0.002151676919311285f, 0.003430439392104745f, 0.002849030075594783f, 0.00228008721023798f, 0.0014942580601200461f, 0.002214002190157771f, 0.0017614219104871154f, 0.0025836704298853874f, 0.0022794180549681187f, 0.002043333137407899f, 0.003510815091431141f, 0.00309583218768239f, 0.0025176377967000008f, 0.0019144181860610843f, 0.0011876773787662387f, 0.004908188711851835f, 0.0015377510571852326f, 0.0016969850985333323f, 0.0055244010873138905f, 0.0026789691764861345f, 0.001076859305612743f, 0.0029507402796298265f, 0.002037427853792906f, 0.0023068131413310766f, 0.002533117774873972f, 0.0013952526496723294f, 0.0025890469551086426f, 0.00221478333696723f, 0.002151061547920108f, 0.0015194276347756386f, 0.002584080910310149f, 0.0032543863635510206f, 0.001770641072653234f, 0.0015419928822666407f, 0.001702605513855815f, 0.0027986024506390095f, 0.0024626876693218946f, 0.0013540189247578382f, 0.002600147156044841f, 0.0027652611024677753f, 0.005235159769654274f, 0.003123514587059617f, 0.0034850675147026777f, 0.004103482235223055f, 0.0015350892208516598f, 0.001392922131344676f, 0.0029379804618656635f, 0.002204966964200139f, 0.0015737969661131501f, 0.0029938609804958105f, 0.0029448734130710363f, 0.002586197108030319f, 0.0023604186717420816f, 0.0050611719489097595f, 0.0023034019395709038f, 0.00164567690808326f, 0.0016214968636631966f, 0.002210998209193349f, 0.002312767319381237f, 0.0021691799629479647f, 0.0034633572213351727f, 0.005106713157147169f, 0.002001560293138027f, 0.001487371395342052f, 0.0017395884497091174f, 0.002040277002379298f, 0.001837808289565146f, 0.002415858209133148f, 0.0017662645550444722f, 0.004152821842581034f, 0.0023349239490926266f, 0.0018082436872646213f, 0.0019227223237976432f, 0.0024141548201441765f, 0.002637046156451106f, 0.0021490028593689203f, 0.0030219247564673424f, 0.002614756114780903f, 0.001981128240004182f, 0.001411812729202211f, 0.002254475373774767f, 0.002039737533777952f, 0.0023458427749574184f, 0.0019043376669287682f);
static const ai_layer_format_type conv2d_13_l_out_ch_format_const_layer_format_type = AI_LAYER_FORMAT_CHANNEL_LAST_VALID;

static const ai_u16 conv2d_14_t_in_0_shape_w_const_u16 = 16;
static const ai_u16 conv2d_14_t_in_0_shape_h_const_u16 = 16;
static const ai_u16 conv2d_14_l_stride_1_const_u16 = 1;
static const ai_u16 conv2d_14_l_stride_0_const_u16 = 1;
static const ai_u16 conv2d_14_t_in_0_shape_ch_const_u16 = 256;
static const ai_u16 conv2d_14_t_out_0_shape_ch_const_u16 = 30;
static const ai_i8 conv2d_14_t_in_0_fmt_zero_const_s8 = -128;
static const ai_i8 conv2d_14_t_out_0_fmt_zero_const_s8 = 50;
static const ai_float conv2d_14_t_in_0_fmt_scale_const_f32 = 0.030627276748418808f;
static const ai_float conv2d_14_t_out_0_fmt_scale_const_f32 = 0.09362839162349701f;
static const ai_float conv2d_14_t_weight_0_fmt_scale_const_f32[] = LITE_ARRAY_VALUES(0.008113464340567589f, 0.0035943116527050734f, 0.001963220536708832f, 0.0026472166646271944f, 0.006451227702200413f, 0.0021435099188238382f, 0.002713270951062441f, 0.0036294551100581884f, 0.0009760424727573991f, 0.0013337214477360249f, 0.004106419160962105f, 0.0015313990879803896f, 0.005743112415075302f, 0.003756684949621558f, 0.0005135006504133344f, 0.0016307936748489738f, 0.0035586499143391848f, 0.0025922851637005806f, 0.003530789166688919f, 0.0026415539905428886f, 0.0003788525646086782f, 0.0007754770340397954f, 0.0038559657987207174f, 0.0015738646034151316f, 0.0035246899351477623f, 0.0034985796082764864f, 0.0036150142550468445f, 0.0025785237085074186f, 0.004737271461635828f, 0.0019598877988755703f);
static const ai_layer_format_type conv2d_14_l_out_ch_format_const_layer_format_type = AI_LAYER_FORMAT_CHANNEL_LAST_VALID;

static const ai_u32 conversion_15_t_out_0_shape_h_w_ch_d_prod_const_u32 = 7680;
static const ai_float conversion_15_t_in_0_fmt_scale_const_f32 = 0.09362839162349701f;
static const ai_i8 conversion_15_t_in_0_fmt_zero_const_s8 = 50;
STAI_API_ENTRY
stai_return_code stai_network_run(
  stai_network* network,
  const stai_run_mode mode)
{
   STAI_UNUSED(mode)
  _STAI_CONTEXT_ACQUIRE(net_ctx, network)

  _STAI_SET_ERROR(net_ctx, (net_ctx->_flags & STAI_FLAG_ACTIVATIONS) != STAI_FLAG_ACTIVATIONS,
        STAI_ERROR_NETWORK_INVALID_ACTIVATIONS_PTR, net_ctx->_return_code)

  _STAI_SET_ERROR(net_ctx, (net_ctx->_flags & STAI_FLAG_INPUTS) != STAI_FLAG_INPUTS,
                  STAI_ERROR_NETWORK_INVALID_IN_PTR, net_ctx->_return_code)
  _STAI_SET_ERROR(net_ctx, (net_ctx->_flags & STAI_FLAG_OUTPUTS) != STAI_FLAG_OUTPUTS,
                  STAI_ERROR_NETWORK_INVALID_OUT_PTR, net_ctx->_return_code)

  _STAI_SET_ERROR(net_ctx, (net_ctx->_flags & STAI_FLAG_WEIGHTS) != STAI_FLAG_WEIGHTS,
                  STAI_ERROR_NETWORK_INVALID_WEIGHTS_PTR, net_ctx->_return_code)


  /* LITE_KERNEL_SECTION BEGIN conversion_0 */
  {
      const ai_u8* conversion_0_t_in_0_ptr_const_u8 = (ai_u8*)(net_ctx->_inputs[0] + 0);
    ai_i8* conversion_0_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 0);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(0, 1, {(stai_ptr) conversion_0_t_in_0_ptr_const_u8});
    
  forward_lite_node_convert_integer_iu8os8(conversion_0_t_in_0_ptr_const_u8, conversion_0_t_out_0_ptr_s8, conversion_0_t_in_0_shape_h_w_ch_d_prod_const_u32, 1.0f, conversion_0_t_in_0_fmt_zero_const_u8, conversion_0_t_out_0_fmt_zero_const_s8);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(0, 1, {(stai_ptr) conversion_0_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conversion_0 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_1 */
  {
      const ai_i8* conv2d_1_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 0);
    const ai_i8* conv2d_1_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 0);
    const ai_i32* conv2d_1_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 432);
    ai_i8* conv2d_1_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 197808);
    ai_i16* conv2d_1_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 196612);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(1, 1, {(stai_ptr) conv2d_1_t_in_0_ptr_const_s8});
    
  forward_lite_conv2d_rgb_sssa8_ch(conv2d_1_t_in_0_ptr_const_s8, conv2d_1_t_in_0_shape_w_const_u16, conv2d_1_t_weight_0_ptr_const_s8, conv2d_1_t_out_0_shape_ch_const_u16, conv2d_1_t_weight_0_shape_w_const_u16, conv2d_1_l_pad_W_0_const_s32, conv2d_1_l_stride_0_const_u16, conv2d_1_t_weight_1_ptr_const_s32, conv2d_1_t_in_0_fmt_zero_const_s8, conv2d_1_t_out_0_fmt_zero_const_s8, conv2d_1_t_in_0_fmt_scale_const_f32, conv2d_1_t_out_0_fmt_scale_const_f32, conv2d_1_t_weight_0_fmt_scale_const_f32, conv2d_1_l_out_ch_format_const_layer_format_type, conv2d_1_t_out_0_ptr_s8, conv2d_1_t_out_0_shape_w_const_u16, 1196, conv2d_1_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(1, 1, {(stai_ptr) conv2d_1_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_1 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_2_pad_before */
  {
      const ai_ptr conv2d_2_pad_before_t_in_0_ptr_const_ptr = (ai_ptr)(net_ctx->_activations[0] + 197808);
    ai_ptr conv2d_2_pad_before_t_out_0_ptr_ptr = (ai_ptr)(net_ctx->_activations[0] + 189552);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(2, 1, {(stai_ptr) conv2d_2_pad_before_t_in_0_ptr_const_ptr});
    
  forward_lite_pad_constant(conv2d_2_pad_before_t_in_0_ptr_const_ptr, conv2d_2_pad_before_t_out_0_ptr_ptr, (ai_handle)(conv2d_2_pad_before_v_pad_constant_value_const_s8), conv2d_2_pad_before_t_in_0_fmt_bitsize_const_s16, conv2d_2_pad_before_t_in_0_shape_h_const_u32, (ai_i32)(1), (ai_i32)(2048), (ai_i32)(0), (ai_i32)(4160), (ai_i32)(0), (ai_i32)(32));
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(2, 1, {(stai_ptr) conv2d_2_pad_before_t_out_0_ptr_ptr});
  }
  /* LITE_KERNEL_SECTION END conv2d_2_pad_before */
  /* LITE_KERNEL_SECTION BEGIN conv2d_2 */
  {
      const ai_i8* conv2d_2_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 189552);
    const ai_i8* conv2d_2_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 496);
    const ai_i32* conv2d_2_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 5104);
    ai_i8* conv2d_2_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 6144);
    ai_i16* conv2d_2_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 0);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(2, 1, {(stai_ptr) conv2d_2_t_in_0_ptr_const_s8});
    
  forward_lite_conv2d_sssa8_ch(conv2d_2_t_in_0_ptr_const_s8, conv2d_2_t_in_0_shape_w_const_u16, conv2d_2_t_in_0_shape_h_const_u16, conv2d_2_t_in_0_shape_ch_const_u16, conv2d_2_t_weight_0_ptr_const_s8, conv2d_2_t_out_0_shape_ch_const_u16, conv2d_2_t_weight_0_shape_w_const_u16, conv2d_2_t_weight_0_shape_h_const_u16, conv2d_2_l_stride_1_const_u16, conv2d_2_l_stride_0_const_u16, conv2d_2_l_pad_W_0_const_s32, conv2d_2_l_pad_H_0_const_s32, conv2d_2_t_weight_1_ptr_const_s32, conv2d_2_t_in_0_fmt_zero_const_s8, conv2d_2_t_out_0_fmt_zero_const_s8, conv2d_2_t_in_0_fmt_scale_const_f32, conv2d_2_t_out_0_fmt_scale_const_f32, conv2d_2_t_weight_0_fmt_scale_const_f32, conv2d_2_l_out_ch_format_const_layer_format_type, conv2d_2_t_out_0_ptr_s8, conv2d_2_t_out_0_shape_w_const_u16, conv2d_2_t_out_0_shape_h_const_u16, 1, 6144, conv2d_2_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(2, 1, {(stai_ptr) conv2d_2_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_2 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_3_pad_before */
  {
      const ai_ptr conv2d_3_pad_before_t_in_0_ptr_const_ptr = (ai_ptr)(net_ctx->_activations[0] + 6144);
    ai_ptr conv2d_3_pad_before_t_out_0_ptr_ptr = (ai_ptr)(net_ctx->_activations[0] + 137216);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(3, 1, {(stai_ptr) conv2d_3_pad_before_t_in_0_ptr_const_ptr});
    
  forward_lite_pad_constant(conv2d_3_pad_before_t_in_0_ptr_const_ptr, conv2d_3_pad_before_t_out_0_ptr_ptr, (ai_handle)(conv2d_3_pad_before_v_pad_constant_value_const_s8), conv2d_3_pad_before_t_in_0_fmt_bitsize_const_s16, conv2d_3_pad_before_t_in_0_shape_h_const_u32, (ai_i32)(1), (ai_i32)(2048), (ai_i32)(0), (ai_i32)(4224), (ai_i32)(0), (ai_i32)(64));
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(3, 1, {(stai_ptr) conv2d_3_pad_before_t_out_0_ptr_ptr});
  }
  /* LITE_KERNEL_SECTION END conv2d_3_pad_before */
  /* LITE_KERNEL_SECTION BEGIN conv2d_3 */
  {
      const ai_i8* conv2d_3_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 137216);
    const ai_i8* conv2d_3_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 5232);
    const ai_i32* conv2d_3_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 23664);
    ai_i8* conv2d_3_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 7168);
    ai_i16* conv2d_3_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 0);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(3, 1, {(stai_ptr) conv2d_3_t_in_0_ptr_const_s8});
    
  forward_lite_conv2d_deep_sssa8_ch(conv2d_3_t_in_0_ptr_const_s8, conv2d_3_t_in_0_shape_w_const_u16, conv2d_3_t_in_0_shape_h_const_u16, conv2d_3_t_in_0_shape_ch_const_u16, conv2d_3_t_weight_0_ptr_const_s8, conv2d_3_t_out_0_shape_ch_const_u16, conv2d_3_t_weight_0_shape_w_const_u16, conv2d_3_t_weight_0_shape_h_const_u16, conv2d_3_l_stride_1_const_u16, conv2d_3_l_stride_0_const_u16, conv2d_3_t_weight_1_ptr_const_s32, conv2d_3_t_in_0_fmt_zero_const_s8, conv2d_3_t_out_0_fmt_zero_const_s8, conv2d_3_t_in_0_fmt_scale_const_f32, conv2d_3_t_out_0_fmt_scale_const_f32, conv2d_3_t_weight_0_fmt_scale_const_f32, conv2d_3_l_out_ch_format_const_layer_format_type, conv2d_3_t_out_0_ptr_s8, conv2d_3_t_out_0_shape_w_const_u16, conv2d_3_t_out_0_shape_h_const_u16, 1, 1, 7168, conv2d_3_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(3, 1, {(stai_ptr) conv2d_3_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_3 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_4_pad_before */
  {
      const ai_ptr conv2d_4_pad_before_t_in_0_ptr_const_ptr = (ai_ptr)(net_ctx->_activations[0] + 7168);
    ai_ptr conv2d_4_pad_before_t_out_0_ptr_ptr = (ai_ptr)(net_ctx->_activations[0] + 72704);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(4, 1, {(stai_ptr) conv2d_4_pad_before_t_in_0_ptr_const_ptr});
    
  forward_lite_pad_constant(conv2d_4_pad_before_t_in_0_ptr_const_ptr, conv2d_4_pad_before_t_out_0_ptr_ptr, (ai_handle)(conv2d_4_pad_before_v_pad_constant_value_const_s8), conv2d_4_pad_before_t_in_0_fmt_bitsize_const_s16, conv2d_4_pad_before_t_in_0_shape_h_const_u32, (ai_i32)(1), (ai_i32)(2048), (ai_i32)(0), (ai_i32)(4352), (ai_i32)(0), (ai_i32)(128));
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(4, 1, {(stai_ptr) conv2d_4_pad_before_t_out_0_ptr_ptr});
  }
  /* LITE_KERNEL_SECTION END conv2d_4_pad_before */
  /* LITE_KERNEL_SECTION BEGIN conv2d_4 */
  {
      const ai_i8* conv2d_4_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 72704);
    const ai_i8* conv2d_4_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 23920);
    const ai_i32* conv2d_4_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 97648);
    ai_i8* conv2d_4_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 9216);
    ai_i16* conv2d_4_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 0);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(4, 1, {(stai_ptr) conv2d_4_t_in_0_ptr_const_s8});
    
  forward_lite_conv2d_deep_sssa8_ch(conv2d_4_t_in_0_ptr_const_s8, conv2d_4_t_in_0_shape_w_const_u16, conv2d_4_t_in_0_shape_h_const_u16, conv2d_4_t_in_0_shape_ch_const_u16, conv2d_4_t_weight_0_ptr_const_s8, conv2d_4_t_out_0_shape_ch_const_u16, conv2d_4_t_weight_0_shape_w_const_u16, conv2d_4_t_weight_0_shape_h_const_u16, conv2d_4_l_stride_1_const_u16, conv2d_4_l_stride_0_const_u16, conv2d_4_t_weight_1_ptr_const_s32, conv2d_4_t_in_0_fmt_zero_const_s8, conv2d_4_t_out_0_fmt_zero_const_s8, conv2d_4_t_in_0_fmt_scale_const_f32, conv2d_4_t_out_0_fmt_scale_const_f32, conv2d_4_t_weight_0_fmt_scale_const_f32, conv2d_4_l_out_ch_format_const_layer_format_type, conv2d_4_t_out_0_ptr_s8, conv2d_4_t_out_0_shape_w_const_u16, conv2d_4_t_out_0_shape_h_const_u16, 1, 1, 9216, conv2d_4_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(4, 1, {(stai_ptr) conv2d_4_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_4 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_5_pad_before */
  {
    
  forward_lite_conv2d_5_pad_before(net_ctx);
  }
  /* LITE_KERNEL_SECTION END conv2d_5_pad_before */
  /* LITE_KERNEL_SECTION BEGIN conv2d_5 */
  {
      const ai_i8* conv2d_5_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 9216);
    const ai_i8* conv2d_5_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 98160);
    const ai_i32* conv2d_5_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 99312);
    ai_i8* conv2d_5_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 50688);
    ai_i16* conv2d_5_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 0);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(5, 1, {(stai_ptr) conv2d_5_t_in_0_ptr_const_s8});
    
  forward_lite_dw_3x3_ch1st_sssa8_ch(conv2d_5_t_in_0_ptr_const_s8, conv2d_5_t_in_0_shape_w_const_u16, conv2d_5_t_in_0_shape_h_const_u16, conv2d_5_t_in_0_shape_ch_const_u16, conv2d_5_t_weight_0_ptr_const_s8, conv2d_5_l_stride_1_const_u16, conv2d_5_l_stride_0_const_u16, conv2d_5_t_weight_1_ptr_const_s32, conv2d_5_t_in_0_fmt_zero_const_s8, conv2d_5_t_out_0_fmt_zero_const_s8, conv2d_5_t_in_0_fmt_scale_const_f32, conv2d_5_t_out_0_fmt_scale_const_f32, conv2d_5_t_weight_0_fmt_scale_const_f32, conv2d_5_t_out_0_ptr_s8, conv2d_5_t_out_0_shape_w_const_u16, conv2d_5_t_out_0_shape_h_const_u16, 0, 4352, conv2d_5_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(5, 1, {(stai_ptr) conv2d_5_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_5 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_6 */
  {
      const ai_i8* conv2d_6_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 50688);
    const ai_i8* conv2d_6_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 99824);
    const ai_i32* conv2d_6_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 132592);
    ai_i8* conv2d_6_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 83456);
    ai_i16* conv2d_6_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 0);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(6, 1, {(stai_ptr) conv2d_6_t_in_0_ptr_const_s8});
    
  forward_lite_pw_sssa8_ch(conv2d_6_t_in_0_ptr_const_s8, conv2d_6_t_in_0_shape_w_const_u16, conv2d_6_t_in_0_shape_h_const_u16, conv2d_6_l_stride_1_const_u16, conv2d_6_l_stride_0_const_u16, conv2d_6_t_in_0_shape_ch_const_u16, conv2d_6_t_weight_0_ptr_const_s8, conv2d_6_t_out_0_shape_ch_const_u16, conv2d_6_t_weight_1_ptr_const_s32, conv2d_6_t_in_0_fmt_zero_const_s8, conv2d_6_t_out_0_fmt_zero_const_s8, conv2d_6_t_in_0_fmt_scale_const_f32, conv2d_6_t_out_0_fmt_scale_const_f32, conv2d_6_t_weight_0_fmt_scale_const_f32, conv2d_6_l_out_ch_format_const_layer_format_type, conv2d_6_t_out_0_ptr_s8, 1, 3072, conv2d_6_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(6, 1, {(stai_ptr) conv2d_6_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_6 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_7_pad_before */
  {
    
  forward_lite_conv2d_7_pad_before(net_ctx);
  }
  /* LITE_KERNEL_SECTION END conv2d_7_pad_before */
  /* LITE_KERNEL_SECTION BEGIN conv2d_7 */
  {
      const ai_i8* conv2d_7_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 83456);
    const ai_i8* conv2d_7_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 133616);
    const ai_i32* conv2d_7_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 135920);
    ai_i8* conv2d_7_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 8704);
    ai_i16* conv2d_7_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 0);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(7, 1, {(stai_ptr) conv2d_7_t_in_0_ptr_const_s8});
    
  forward_lite_dw_3x3_ch1st_sssa8_ch(conv2d_7_t_in_0_ptr_const_s8, conv2d_7_t_in_0_shape_w_const_u16, conv2d_7_t_in_0_shape_h_const_u16, conv2d_7_t_in_0_shape_ch_const_u16, conv2d_7_t_weight_0_ptr_const_s8, conv2d_7_l_stride_1_const_u16, conv2d_7_l_stride_0_const_u16, conv2d_7_t_weight_1_ptr_const_s32, conv2d_7_t_in_0_fmt_zero_const_s8, conv2d_7_t_out_0_fmt_zero_const_s8, conv2d_7_t_in_0_fmt_scale_const_f32, conv2d_7_t_out_0_fmt_scale_const_f32, conv2d_7_t_weight_0_fmt_scale_const_f32, conv2d_7_t_out_0_ptr_s8, conv2d_7_t_out_0_shape_w_const_u16, conv2d_7_t_out_0_shape_h_const_u16, 0, 8704, conv2d_7_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(7, 1, {(stai_ptr) conv2d_7_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_7 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_8 */
  {
      const ai_i8* conv2d_8_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 8704);
    const ai_i8* conv2d_8_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 136944);
    const ai_i32* conv2d_8_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 202480);
    ai_i8* conv2d_8_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 74240);
    ai_i16* conv2d_8_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 0);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(8, 1, {(stai_ptr) conv2d_8_t_in_0_ptr_const_s8});
    
  forward_lite_pw_sssa8_ch(conv2d_8_t_in_0_ptr_const_s8, conv2d_8_t_in_0_shape_w_const_u16, conv2d_8_t_in_0_shape_h_const_u16, conv2d_8_l_stride_1_const_u16, conv2d_8_l_stride_0_const_u16, conv2d_8_t_in_0_shape_ch_const_u16, conv2d_8_t_weight_0_ptr_const_s8, conv2d_8_t_out_0_shape_ch_const_u16, conv2d_8_t_weight_1_ptr_const_s32, conv2d_8_t_in_0_fmt_zero_const_s8, conv2d_8_t_out_0_fmt_zero_const_s8, conv2d_8_t_in_0_fmt_scale_const_f32, conv2d_8_t_out_0_fmt_scale_const_f32, conv2d_8_t_weight_0_fmt_scale_const_f32, conv2d_8_l_out_ch_format_const_layer_format_type, conv2d_8_t_out_0_ptr_s8, 1, 3584, conv2d_8_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(8, 1, {(stai_ptr) conv2d_8_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_8 */
  /* LITE_KERNEL_SECTION BEGIN pool_9 */
  {
      const ai_i8* pool_9_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 74240);
    ai_i8* pool_9_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 0);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(9, 1, {(stai_ptr) pool_9_t_in_0_ptr_const_s8});
    
  forward_lite_maxpool_is8os8_scalepos(pool_9_t_in_0_ptr_const_s8, pool_9_t_out_0_ptr_s8, pool_9_t_in_0_shape_w_const_u16, pool_9_t_in_0_shape_h_const_u16, pool_9_t_in_0_shape_ch_const_u16, pool_9_l_pool_size_1_const_u16, pool_9_l_pool_size_0_const_u16, pool_9_l_legacy_pool_pad_1_const_u16, pool_9_l_legacy_pool_pad_0_const_u16, pool_9_l_pool_stride_1_const_u16, pool_9_l_pool_stride_0_const_u16, pool_9_t_out_0_shape_w_const_u16, pool_9_t_out_0_shape_h_const_u16, 1.0f, pool_9_t_in_0_fmt_zero_const_s8, pool_9_t_out_0_fmt_zero_const_s8);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(9, 1, {(stai_ptr) pool_9_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END pool_9 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_10_pad_before */
  {
      const ai_ptr conv2d_10_pad_before_t_in_0_ptr_const_ptr = (ai_ptr)(net_ctx->_activations[0] + 0);
    ai_ptr conv2d_10_pad_before_t_out_0_ptr_ptr = (ai_ptr)(net_ctx->_activations[0] + 65536);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(10, 1, {(stai_ptr) conv2d_10_pad_before_t_in_0_ptr_const_ptr});
    
  forward_lite_pad_constant(conv2d_10_pad_before_t_in_0_ptr_const_ptr, conv2d_10_pad_before_t_out_0_ptr_ptr, (ai_handle)(conv2d_10_pad_before_v_pad_constant_value_const_s8), conv2d_10_pad_before_t_in_0_fmt_bitsize_const_s16, conv2d_10_pad_before_t_in_0_shape_h_const_u32, (ai_i32)(1), (ai_i32)(4096), (ai_i32)(4608), (ai_i32)(4608), (ai_i32)(256), (ai_i32)(256));
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(10, 1, {(stai_ptr) conv2d_10_pad_before_t_out_0_ptr_ptr});
  }
  /* LITE_KERNEL_SECTION END conv2d_10_pad_before */
  /* LITE_KERNEL_SECTION BEGIN conv2d_10 */
  {
      const ai_i8* conv2d_10_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 65536);
    const ai_i8* conv2d_10_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 203504);
    const ai_i32* conv2d_10_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 205808);
    ai_i8* conv2d_10_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 148480);
    ai_i16* conv2d_10_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 0);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(10, 1, {(stai_ptr) conv2d_10_t_in_0_ptr_const_s8});
    
  forward_lite_dw_3x3_sssa8_ch(conv2d_10_t_in_0_ptr_const_s8, conv2d_10_t_in_0_shape_w_const_u16, conv2d_10_t_in_0_shape_h_const_u16, conv2d_10_t_in_0_shape_ch_const_u16, conv2d_10_t_weight_0_ptr_const_s8, conv2d_10_l_stride_1_const_u16, conv2d_10_l_stride_0_const_u16, conv2d_10_t_weight_1_ptr_const_s32, conv2d_10_t_in_0_fmt_zero_const_s8, conv2d_10_t_out_0_fmt_zero_const_s8, conv2d_10_t_in_0_fmt_scale_const_f32, conv2d_10_t_out_0_fmt_scale_const_f32, conv2d_10_t_weight_0_fmt_scale_const_f32, conv2d_10_t_out_0_ptr_s8, conv2d_10_t_out_0_shape_w_const_u16, conv2d_10_t_out_0_shape_h_const_u16, 0, 9473, conv2d_10_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(10, 1, {(stai_ptr) conv2d_10_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_10 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_11 */
  {
      const ai_i8* conv2d_11_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 148480);
    const ai_i8* conv2d_11_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 206832);
    const ai_i32* conv2d_11_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 239600);
    ai_i8* conv2d_11_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 2304);
    ai_i16* conv2d_11_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 0);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(11, 1, {(stai_ptr) conv2d_11_t_in_0_ptr_const_s8});
    
  forward_lite_pw_sssa8_ch(conv2d_11_t_in_0_ptr_const_s8, conv2d_11_t_in_0_shape_w_const_u16, conv2d_11_t_in_0_shape_h_const_u16, conv2d_11_l_stride_1_const_u16, conv2d_11_l_stride_0_const_u16, conv2d_11_t_in_0_shape_ch_const_u16, conv2d_11_t_weight_0_ptr_const_s8, conv2d_11_t_out_0_shape_ch_const_u16, conv2d_11_t_weight_1_ptr_const_s32, conv2d_11_t_in_0_fmt_zero_const_s8, conv2d_11_t_out_0_fmt_zero_const_s8, conv2d_11_t_in_0_fmt_scale_const_f32, conv2d_11_t_out_0_fmt_scale_const_f32, conv2d_11_t_weight_0_fmt_scale_const_f32, conv2d_11_l_out_ch_format_const_layer_format_type, conv2d_11_t_out_0_ptr_s8, 1, 2304, conv2d_11_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(11, 1, {(stai_ptr) conv2d_11_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_11 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_12_pad_before */
  {
    
  forward_lite_conv2d_12_pad_before(net_ctx);
  }
  /* LITE_KERNEL_SECTION END conv2d_12_pad_before */
  /* LITE_KERNEL_SECTION BEGIN conv2d_12 */
  {
      const ai_i8* conv2d_12_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 2304);
    const ai_i8* conv2d_12_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 240112);
    const ai_i32* conv2d_12_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 241264);
    ai_i8* conv2d_12_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 48128);
    ai_i16* conv2d_12_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 43776);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(12, 1, {(stai_ptr) conv2d_12_t_in_0_ptr_const_s8});
    
  forward_lite_dw_3x3_ch1st_sssa8_ch(conv2d_12_t_in_0_ptr_const_s8, conv2d_12_t_in_0_shape_w_const_u16, conv2d_12_t_in_0_shape_h_const_u16, conv2d_12_t_in_0_shape_ch_const_u16, conv2d_12_t_weight_0_ptr_const_s8, conv2d_12_l_stride_1_const_u16, conv2d_12_l_stride_0_const_u16, conv2d_12_t_weight_1_ptr_const_s32, conv2d_12_t_in_0_fmt_zero_const_s8, conv2d_12_t_out_0_fmt_zero_const_s8, conv2d_12_t_in_0_fmt_scale_const_f32, conv2d_12_t_out_0_fmt_scale_const_f32, conv2d_12_t_weight_0_fmt_scale_const_f32, conv2d_12_t_out_0_ptr_s8, conv2d_12_t_out_0_shape_w_const_u16, conv2d_12_t_out_0_shape_h_const_u16, 0, 4352, conv2d_12_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(12, 1, {(stai_ptr) conv2d_12_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_12 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_13 */
  {
      const ai_i8* conv2d_13_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 48128);
    const ai_i8* conv2d_13_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 241776);
    const ai_i32* conv2d_13_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 274544);
    ai_i8* conv2d_13_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 80896);
    ai_i16* conv2d_13_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 0);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(13, 1, {(stai_ptr) conv2d_13_t_in_0_ptr_const_s8});
    
  forward_lite_pw_sssa8_ch(conv2d_13_t_in_0_ptr_const_s8, conv2d_13_t_in_0_shape_w_const_u16, conv2d_13_t_in_0_shape_h_const_u16, conv2d_13_l_stride_1_const_u16, conv2d_13_l_stride_0_const_u16, conv2d_13_t_in_0_shape_ch_const_u16, conv2d_13_t_weight_0_ptr_const_s8, conv2d_13_t_out_0_shape_ch_const_u16, conv2d_13_t_weight_1_ptr_const_s32, conv2d_13_t_in_0_fmt_zero_const_s8, conv2d_13_t_out_0_fmt_zero_const_s8, conv2d_13_t_in_0_fmt_scale_const_f32, conv2d_13_t_out_0_fmt_scale_const_f32, conv2d_13_t_weight_0_fmt_scale_const_f32, conv2d_13_l_out_ch_format_const_layer_format_type, conv2d_13_t_out_0_ptr_s8, 1, 3072, conv2d_13_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(13, 1, {(stai_ptr) conv2d_13_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_13 */
  /* LITE_KERNEL_SECTION BEGIN conv2d_14 */
  {
      const ai_i8* conv2d_14_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 80896);
    const ai_i8* conv2d_14_t_weight_0_ptr_const_s8 = (ai_i8*)(net_ctx->_weights[0] + 275568);
    const ai_i32* conv2d_14_t_weight_1_ptr_const_s32 = (ai_i32*)(net_ctx->_weights[0] + 283248);
    ai_i8* conv2d_14_t_out_0_ptr_s8 = (ai_i8*)(net_ctx->_activations[0] + 1324);
    ai_i16* conv2d_14_t_scratch_0_ptr_s16 = (ai_i16*)(net_ctx->_activations[0] + 0);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(14, 1, {(stai_ptr) conv2d_14_t_in_0_ptr_const_s8});
    
  forward_lite_pw_sssa8_ch(conv2d_14_t_in_0_ptr_const_s8, conv2d_14_t_in_0_shape_w_const_u16, conv2d_14_t_in_0_shape_h_const_u16, conv2d_14_l_stride_1_const_u16, conv2d_14_l_stride_0_const_u16, conv2d_14_t_in_0_shape_ch_const_u16, conv2d_14_t_weight_0_ptr_const_s8, conv2d_14_t_out_0_shape_ch_const_u16, conv2d_14_t_weight_1_ptr_const_s32, conv2d_14_t_in_0_fmt_zero_const_s8, conv2d_14_t_out_0_fmt_zero_const_s8, conv2d_14_t_in_0_fmt_scale_const_f32, conv2d_14_t_out_0_fmt_scale_const_f32, conv2d_14_t_weight_0_fmt_scale_const_f32, conv2d_14_l_out_ch_format_const_layer_format_type, conv2d_14_t_out_0_ptr_s8, 1, 1324, conv2d_14_t_scratch_0_ptr_s16);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(14, 1, {(stai_ptr) conv2d_14_t_out_0_ptr_s8});
  }
  /* LITE_KERNEL_SECTION END conv2d_14 */
  /* LITE_KERNEL_SECTION BEGIN conversion_15 */
  {
      const ai_i8* conversion_15_t_in_0_ptr_const_s8 = (ai_i8*)(net_ctx->_activations[0] + 1324);
    ai_float* conversion_15_t_out_0_ptr_f32 = (ai_float*)(net_ctx->_outputs[0] + 0);
  
  _STAI_NETWORK_EVENT_NODE_START_CB(15, 1, {(stai_ptr) conversion_15_t_in_0_ptr_const_s8});
    
  forward_lite_node_convert_integer_is8of32(conversion_15_t_in_0_ptr_const_s8, conversion_15_t_out_0_ptr_f32, conversion_15_t_out_0_shape_h_w_ch_d_prod_const_u32, conversion_15_t_in_0_fmt_scale_const_f32, conversion_15_t_in_0_fmt_zero_const_s8);
    
  _STAI_NETWORK_EVENT_NODE_STOP_CB(15, 1, {(stai_ptr) conversion_15_t_out_0_ptr_f32});
  }
  /* LITE_KERNEL_SECTION END conversion_15 */
  return net_ctx->_return_code;
}

/*****************************************************************************/
/*  Getters APIs Section  */
STAI_API_ENTRY
stai_size stai_network_get_context_size()
{
  return (stai_size)STAI_NETWORK_CONTEXT_SIZE;
}

#if defined(HAVE_NETWORK_INFO)
STAI_API_ENTRY
stai_return_code stai_network_get_info(
  stai_network* network,
  stai_network_info* info)
{
  _STAI_CONTEXT_ACQUIRE(net_ctx, network)
  _STAI_SET_ERROR(net_ctx, info==NULL, STAI_ERROR_NETWORK_INVALID_INFO, net_ctx->_return_code)

  // Copy of network info struct
  *info = g_network_info;

  return STAI_SUCCESS;
}
#endif


STAI_API_ENTRY
stai_return_code stai_network_get_activations(
  stai_network* network, stai_ptr* activations, stai_size* n_activations)
{
  _STAI_CONTEXT_ACQUIRE(net_ctx, network)

  _STAI_SET_ERROR(net_ctx, !n_activations, STAI_ERROR_NETWORK_INVALID_API_ARGUMENTS, net_ctx->_return_code)
  *n_activations = STAI_NETWORK_ACTIVATIONS_NUM;
for (stai_size idx=0; activations && (idx<STAI_NETWORK_ACTIVATIONS_NUM); idx++) {
    // get address of the activations buffers
    activations[idx] = net_ctx->_activations[idx];
  }return net_ctx->_return_code;
}


STAI_API_ENTRY
stai_return_code stai_network_get_weights(
  stai_network* network, stai_ptr* weights, stai_size* n_weights)
{
  _STAI_CONTEXT_ACQUIRE(net_ctx, network)
  _STAI_SET_ERROR(net_ctx, !n_weights, STAI_ERROR_NETWORK_INVALID_API_ARGUMENTS, net_ctx->_return_code)
  *n_weights = STAI_NETWORK_WEIGHTS_NUM;
for (stai_size idx=0; weights && (idx<STAI_NETWORK_WEIGHTS_NUM); idx++) {
    // get address of the weights buffers
    weights[idx] = net_ctx->_weights[idx];
  }return net_ctx->_return_code;
}


STAI_API_ENTRY
stai_return_code stai_network_get_inputs(
  stai_network* network, stai_ptr* inputs, stai_size* n_inputs)
{
  _STAI_CONTEXT_ACQUIRE(net_ctx, network)
  _STAI_SET_ERROR(net_ctx, !n_inputs, STAI_ERROR_NETWORK_INVALID_API_ARGUMENTS, net_ctx->_return_code)
  *n_inputs = STAI_NETWORK_IN_NUM;
  for (stai_size idx=0; inputs && (idx<STAI_NETWORK_IN_NUM); idx++) {
    inputs[idx] = net_ctx->_inputs[idx];
  }
  return net_ctx->_return_code;
}


STAI_API_ENTRY
stai_return_code stai_network_get_outputs(
  stai_network* network, stai_ptr* outputs, stai_size* n_outputs)
{
  _STAI_CONTEXT_ACQUIRE(net_ctx, network)
  _STAI_SET_ERROR(net_ctx, !n_outputs, STAI_ERROR_NETWORK_INVALID_API_ARGUMENTS, net_ctx->_return_code)
  *n_outputs = STAI_NETWORK_OUT_NUM;
  for (stai_size idx=0; outputs && (idx<STAI_NETWORK_OUT_NUM); idx++) {
    outputs[idx] = net_ctx->_outputs[idx];
  }
  return net_ctx->_return_code;
}


STAI_API_ENTRY
stai_return_code stai_network_get_error(
  stai_network* network)
{
  _STAI_CONTEXT_ACQUIRE(net_ctx, network)

  /* return 1st generated error or STAI_SUCCESS if no errors so far */
  return net_ctx->_return_code;
}


STAI_API_ENTRY
stai_return_code stai_network_get_states(
  stai_network* network, stai_ptr* states, stai_size* n_states)
{
  _STAI_CONTEXT_ACQUIRE(net_ctx, network)
  _STAI_SET_ERROR(net_ctx, !n_states, STAI_ERROR_NETWORK_INVALID_API_ARGUMENTS, net_ctx->_return_code)
  /* get the number of internals states (supporting multi-heap also for internal states) */
  *n_states = STAI_NETWORK_STATES_NUM;

  STAI_UNUSED(states)
return net_ctx->_return_code;
}


/*****************************************************************************/
/*  Setters APIs Section  */

STAI_API_ENTRY
stai_return_code stai_network_set_activations(
  stai_network* network,
  const stai_ptr* activations,
  const stai_size n_activations)
{
  _STAI_CONTEXT_ACQUIRE(net_ctx, network)
const uintptr_t _activations_alignment[] = STAI_NETWORK_ACTIVATIONS_ALIGNMENTS;
  STAI_PRINT("  [stai_network_set_activations] network(%p) activations[%d]: %p\n\n", net_ctx, n_activations, activations)
  _STAI_SET_ERROR(net_ctx, !activations,
                  STAI_ERROR_NETWORK_INVALID_API_ARGUMENTS, net_ctx->_return_code)
  _STAI_SET_ERROR(net_ctx, n_activations!=STAI_NETWORK_ACTIVATIONS_NUM,
                  STAI_ERROR_NETWORK_INVALID_ACTIVATIONS_NUM, net_ctx->_return_code)

  for (stai_size idx=0; activations && idx<STAI_NETWORK_ACTIVATIONS_NUM; idx++) {
    STAI_PRINT("  activation[%d]: %p\n", idx, activations[idx])
    _STAI_SET_ERROR(net_ctx, activations[idx]==NULL,
                    STAI_ERROR_NETWORK_INVALID_ACTIVATIONS_PTR, net_ctx->_return_code)
    _STAI_SET_ERROR(net_ctx, ((uintptr_t)activations[idx]) & (_activations_alignment[idx]-1),
                    STAI_ERROR_INVALID_BUFFER_ALIGNMENT, net_ctx->_return_code)
    net_ctx->_activations[idx] = activations[idx];
  }
  net_ctx->_inputs[0] = activations[0] + 4;

  net_ctx->_outputs[0] = activations[0] + 9004;
_stai_network_check(net_ctx);
  return net_ctx->_return_code;
}


STAI_API_ENTRY
stai_return_code stai_network_set_weights(
  stai_network* network,
  const stai_ptr* weights,
  const stai_size n_weights)
{
  _STAI_CONTEXT_ACQUIRE(net_ctx, network)
const uintptr_t _weights_alignment[] = STAI_NETWORK_WEIGHTS_ALIGNMENTS;
  _STAI_SET_ERROR(net_ctx, !weights,
                  STAI_ERROR_NETWORK_INVALID_API_ARGUMENTS, net_ctx->_return_code)
  _STAI_SET_ERROR(net_ctx, n_weights!=STAI_NETWORK_WEIGHTS_NUM,
                  STAI_ERROR_NETWORK_INVALID_WEIGHTS_NUM, net_ctx->_return_code)
  for (stai_size idx=0; weights && idx<STAI_NETWORK_WEIGHTS_NUM; idx++) {
    STAI_PRINT("  weight[%d]: %p\n", idx, weights[idx])
    _STAI_SET_ERROR(net_ctx, weights[idx]==NULL,
                    STAI_ERROR_NETWORK_INVALID_WEIGHTS_PTR, net_ctx->_return_code)
    _STAI_SET_ERROR(net_ctx, ((uintptr_t)weights[idx]) & (_weights_alignment[idx]-1),
                    STAI_ERROR_INVALID_BUFFER_ALIGNMENT, net_ctx->_return_code)
    net_ctx->_weights[idx] = weights[idx];
  }_stai_network_check(net_ctx);
  return net_ctx->_return_code;
}


STAI_API_ENTRY
stai_return_code stai_network_set_inputs(
  stai_network* network,
  const stai_ptr* inputs,
  const stai_size n_inputs)
{
  const uintptr_t _inputs_alignment[] = STAI_NETWORK_IN_ALIGNMENTS;
  _STAI_CONTEXT_ACQUIRE(net_ctx, network)
  _STAI_SET_ERROR(net_ctx, !inputs,
                  STAI_ERROR_NETWORK_INVALID_API_ARGUMENTS, net_ctx->_return_code)
  _STAI_SET_ERROR(net_ctx, n_inputs!=STAI_NETWORK_IN_NUM,
                  STAI_ERROR_NETWORK_INVALID_IN_NUM, net_ctx->_return_code)

  for (stai_size idx=0; inputs && idx<STAI_NETWORK_IN_NUM; idx++) {
    STAI_PRINT("  input[%d]: %p\n", idx, inputs[idx])
    _STAI_SET_ERROR(net_ctx, inputs[idx]==NULL,
                    STAI_ERROR_NETWORK_INVALID_IN_PTR, net_ctx->_return_code)
    _STAI_SET_ERROR(net_ctx, ((uintptr_t)inputs[idx]) & (_inputs_alignment[idx]-1),
                    STAI_ERROR_INVALID_BUFFER_ALIGNMENT, net_ctx->_return_code)
    net_ctx->_inputs[idx] = inputs[idx];
  }

  _stai_network_check(net_ctx);
  return net_ctx->_return_code;
}


STAI_API_ENTRY
stai_return_code stai_network_set_outputs(
  stai_network* network,
  const stai_ptr* outputs,
  const stai_size n_outputs)
{
  const uintptr_t _outputs_alignment[] = STAI_NETWORK_OUT_ALIGNMENTS;
  _STAI_CONTEXT_ACQUIRE(net_ctx, network)
  _STAI_SET_ERROR(net_ctx, !outputs,
                  STAI_ERROR_NETWORK_INVALID_API_ARGUMENTS, net_ctx->_return_code)
  _STAI_SET_ERROR(net_ctx, n_outputs!=STAI_NETWORK_OUT_NUM,
                  STAI_ERROR_NETWORK_INVALID_OUT_NUM, net_ctx->_return_code)

  for (stai_size idx=0; outputs && idx<n_outputs; idx++) {
    STAI_PRINT("  output[%d]: %p\n", idx, outputs[idx])
    _STAI_SET_ERROR(net_ctx, outputs[idx]==NULL,
                    STAI_ERROR_NETWORK_INVALID_OUT_PTR, net_ctx->_return_code)
    _STAI_SET_ERROR(net_ctx, ((uintptr_t)outputs[idx]) & (_outputs_alignment[idx]-1),
                    STAI_ERROR_INVALID_BUFFER_ALIGNMENT, net_ctx->_return_code)
    net_ctx->_outputs[idx] = outputs[idx];
  }

  _stai_network_check(net_ctx);
  return net_ctx->_return_code;
}


STAI_API_ENTRY
stai_return_code stai_network_set_states(
  stai_network* network,
  const stai_ptr* states,
  const stai_size n_states)
{
  _STAI_CONTEXT_ACQUIRE(net_ctx, network)

  STAI_UNUSED(states)
  STAI_UNUSED(n_states)
_stai_network_check(net_ctx);
  return net_ctx->_return_code;
}

STAI_API_ENTRY
stai_return_code stai_network_set_callback(
  stai_network* network, const stai_event_cb cb, void* cb_cookie)
{
  _STAI_CONTEXT_ACQUIRE(net_ctx, network)
  STAI_PRINT("  set_callback %p cb %p cookie %p\n", net_ctx, cb, cb_cookie)
  // _STAI_SET_ERROR(net_ctx, cb==NULL, STAI_ERROR_NETWORK_INVALID_CALLBACK, net_ctx->_return_code)
  net_ctx->_callback = cb;
  net_ctx->_callback_cookie = cb_cookie;
  return net_ctx->_return_code;
}

#undef _STAI_SET_ERROR
#undef _STAI_CONTEXT_ALIGNMENT
#undef _STAI_CONTEXT_ACQUIRE
#undef _STAI_NETWORK_EVENT_NODE_START_CB
#undef _STAI_NETWORK_EVENT_NODE_STOP_CB
#undef _STAI_NETWORK_MODEL_SIGNATURE
#undef _STAI_NETWORK_DATETIME
#undef _STAI_NETWORK_COMPILE_DATETIME

