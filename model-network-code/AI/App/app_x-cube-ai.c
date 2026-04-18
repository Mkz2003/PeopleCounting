
/**
  ******************************************************************************
  * @file    app_x-cube-ai.c
  * @author  X-CUBE-AI C code generator
  * @brief   AI program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

  /**
    * Description
    * v1.0: Minimum template to show how to use the Embedded Client API ST-AI 
    *
        */

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#if defined ( __ICCARM__ )
#elif defined ( __CC_ARM ) || ( __GNUC__ )
#endif

/* System headers */
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#include "app_x-cube-ai.h"
#include "stai.h"



/* USER CODE BEGIN includes */

#include "MyCode.h"
#include "network_data.h"

/* USER CODE END includes */

/* IO buffers ----------------------------------------------------------------*/


/* Input defs ----------------------------------------------------------------*/

/**
STAI_ALIGNED(32) static uint8_t data_in_1[STAI_NETWORK_IN_1_SIZE_BYTES];

// Array to store the data of the input tensor
stai_ptr data_ins[] = {
  data_in_1
}; 
*/

/* Output defs ----------------------------------------------------------------*/

/**
STAI_ALIGNED(32) 
static uint8_t data_out_1[STAI_NETWORK_OUT_1_SIZE_BYTES];

// c-array to store the data of the output tensor
stai_ptr data_outs[] = {
  data_out_1
}; 
*/



/* Global byte buffer to save instantiated C-model network context */
STAI_NETWORK_CONTEXT_DECLARE(network_context, STAI_NETWORK_CONTEXT_SIZE)

/* Activations buffers -------------------------------------------------------*/
STAI_ALIGNED(32) 
__attribute__((section(".sdram_bss"))) static uint8_t heap_overlay_pool[STAI_NETWORK_ACTIVATION_1_SIZE_BYTES];


/* Global c-array to handle the activations buffer */
stai_ptr data_activations[] = { heap_overlay_pool };

STAI_ALIGNED(32) static uint8_t states_1[4];
stai_ptr data_states[] = {
    states_1
};


/* Entry points --------------------------------------------------------------*/
/* Array of pointer to manage the model's input/output tensors */
static stai_size _in_size;
static stai_ptr stai_input[STAI_NETWORK_IN_NUM];
static stai_size _out_size;
static stai_ptr stai_output[STAI_NETWORK_OUT_NUM];

/* 
 * Bootstrap
 */
int aiInit(void) {
  stai_return_code ret_code;
  
  /* 1: Initialize runtime library */
  ret_code = stai_runtime_init();
  if (ret_code != STAI_SUCCESS) { 
    // handle error
  };

  /* 2: Initialize network model context */
  ret_code = stai_network_init(network_context);
  if (ret_code != STAI_SUCCESS) { 
    // handle error
  };

  /* 3: Set network activations buffers */
  ret_code = stai_network_set_activations(network_context, data_activations, STAI_NETWORK_ACTIVATIONS_NUM);
  if (ret_code != STAI_SUCCESS) { 
    // handle error
  };

  /* 4: Update the AI input/output buffers */
  /** Set network input/output buffers 
    * If the model uses no-inputs-allocation or no-outputs-allocation, the addresses of the input/output buffers
    * must be set before running the inference.
    * See https://stedgeai-dc.com/assets/embedded-docs/embedded_client_stai_api.html#ref_api_set_io
    * for more details
    */

  // current model uses allocate-inputs, use this part to overwrite the addresses of the input buffers
  /**
  ret_code = stai_network_set_inputs(network_context, data_ins, STAI_NETWORK_IN_NUM);
  if (ret_code != STAI_SUCCESS) { 
    // handle error
  };
   */
  // current model uses allocate-outputs, use this part to overwrite the addresses of the output buffers
  /** 
  ret_code = stai_network_set_outputs(network_context, data_outs, STAI_NETWORK_OUT_NUM);
  if (ret_code != STAI_SUCCESS) { 
    // handle error
  };
   */

  ret_code = stai_network_get_inputs(network_context, stai_input, &_in_size);
  if (ret_code != STAI_SUCCESS) {
      // handle error
  }

  ret_code = stai_network_get_outputs(network_context, stai_output, &_out_size);
  if (ret_code != STAI_SUCCESS) {
      // handle error
  }
  return 0;
}

int aiDeinit(void) {
  stai_return_code ret_code;

  /* 1: Deinitialize network model context */
  ret_code = stai_network_deinit(network_context);
  if (ret_code != STAI_SUCCESS) { 
    // handle error
  };

  /* 2: Deinitialize runtime library */
  ret_code = stai_runtime_deinit();
  if (ret_code != STAI_SUCCESS) { 
    // handle error
  };

  return 0;
}

/* 
 * Run inference
 */
int aiRun() {

  stai_return_code ret_code;

  /* Perform the inference */
  ret_code = stai_network_run(network_context, STAI_MODE_SYNC);
  if (ret_code != STAI_SUCCESS) {
      ret_code = stai_network_get_error(network_context);
      // handle error
  };
  
  return 0;
}


int acquire_and_process_data()
{
  extern uint16_t* pCamera_Image; // 摄像头数据缓冲区指针
  extern OV5640_FrameState ov5640_FrameState;

  #define ResizeWidth  (STAI_NETWORK_IN_1_WIDTH)
  #define ResizeHeight   (Display_Height * STAI_NETWORK_IN_1_HEIGHT / Display_Width)

  __attribute__((section(".sdram_bss"), aligned(32))) static uint16_t resizedImage[ResizeHeight][ResizeWidth];
  // __attribute__((section(".sdram_bss"), aligned(32))) static uint16_t resizedImage[STAI_NETWORK_IN_1_WIDTH][STAI_NETWORK_IN_1_HEIGHT];
  image_t srcImg, resizedImg; 
  // 检测缓冲区是否切换，同时储存图像
  do
  {
    ov5640_FrameState.objectDetectState = 0;
    STM32Ipl_Init(&srcImg, Display_Width, Display_Height, IMAGE_BPP_RGB565, (uint8_t*)pCamera_Image);

    STM32Ipl_Init(&resizedImg, ResizeWidth, ResizeHeight, IMAGE_BPP_RGB565, resizedImage);
    // STM32Ipl_Init(&resizedImg, STAI_NETWORK_IN_1_HEIGHT, STAI_NETWORK_IN_1_WIDTH, IMAGE_BPP_RGB565, resizedImage);

    STM32Ipl_Resize(&srcImg, &resizedImg, RESIZE_NEAREST);
  } while(ov5640_FrameState.objectDetectState && 0);

  // STM32Ipl_Rotation90(&resizedImg, &resizedImg);

  const size_t offset = (sizeof(STAI_FORMAT) * 3) * STAI_NETWORK_IN_1_WIDTH * (STAI_NETWORK_IN_1_HEIGHT - ResizeHeight) / 2;
  // const size_t offset = 0;
  DMA2D_Fill((uint8_t*)stai_input[0], 
              STAI_NETWORK_IN_1_WIDTH, 
              STAI_NETWORK_IN_1_HEIGHT, 
              0,
              DMA2D_OUTPUT_RGB888, 
              0x00111111
            );
  DMA2D_FormatConvert(resizedImage, 
                        (uint8_t*)stai_input[0] + offset, 
                        resizedImg.w, 
                        resizedImg.h, 
                        0, 
                        0, 
                        DMA2D_INPUT_RGB565, 
                        DMA2D_OUTPUT_RGB888,
                        0
                      );

  // 转换图像为int8
  // for(size_t i = 0; i < ResizeHeight * ResizeWidth * 3; i++) *((ai_i8*)stai_input[0] + offset + i) = rgb888_To_ai_i8(*((uint8_t*)stai_input[0] + offset + i));
  // for(size_t i = 0; i < ResizeHeight * ResizeWidth * 3; i++) *((int8_t*)stai_input[0] + offset + i) = ai_float_32_To_ai_i8(*((uint8_t*)stai_input[0] + offset + i) / 255.0f);

  #undef ResizeHeight
  #undef ResizeWidth 

  return 0;
}

int post_process()
{
  extern array_t* faces,  * eyes, * faceIDs;
  CHANNEL outData = {.score = (void*)stai_output[0],
                      .code = (void*)stai_output[1]};
  array_clear(faces);
  // int32_t label = 1;
  // for(int i = 0; i < STAI_NETWORK_OUT_1_CHANNEL; i++)
  // {
  //   // 找认定label类为最大概率的检测框
  //   uint8_t flag = 0;
  //   for(int j = 0; j < STAI_NETWORK_OUT_1_HEIGHT; j++)
  //   {
  //     if(j != label && ai_i8_To_ai_float(outData->object[label][i]) <= ai_i8_To_ai_float(outData->object[j][i]))
  //     {
  //       flag = 1;
  //       break;
  //     }
  //   }
  //   if(flag || ai_i8_To_ai_float(outData->object[label][i]) <= 0.1f) continue;

  //   // 存入归一化数据
  //   faceObject_t* f = (faceObject_t*)xalloc(sizeof(faceObject_t));

  //   f->rect.x = (*outData.code)[i].x1;
  //   f->rect.y = (*outData.code)[i].y1;
  //   f->rect.width = (*outData.code)[i].x2 - (*outData.code)[i].x1;
  //   f->rect.height = (*outData.code)[i].y2 - (*outData.code)[i].y1;
  //   f->label = label;
  //   f->prob = (*outData.score)[i].person;

  //   array_push_back(faces, f);
  // }
  // get(faces, faceIDs);
  int label = 1;
  int maxl = 0;
  for(int i = 0; i < STAI_NETWORK_OUT_1_CHANNEL; i++)
  {
    if((*outData.score)[i][label] > (*outData.score)[maxl][label]) maxl = i;
  }
  printf("%.2f, %.2f, %.2f, %.2f, %.2f\n",
    (*outData.code)[maxl].x1,
    (*outData.code)[maxl].y1,
    (*outData.code)[maxl].x2,
    (*outData.code)[maxl].y2,
    (*outData.score)[maxl][label]
  );
  return 0;
}



/* 
 * Example of main loop function
 */
void main_loop() {
    /* 1 - Acquire, pre-process and fill the input buffers */
    acquire_and_process_data();

    /* 2 - Call inference engine */
    aiRun();

    /* 3 - Post-process the predictions */
    post_process();
}


/* Entry points --------------------------------------------------------------*/


void STM32CubeAI_Studio_AI_Init(void)
{
    /* USER CODE BEGIN 5 */
    memcpy(g_network_weights_array, _g_network_weights_array, sizeof(_g_network_weights_array));
    aiInit();
    /* USER CODE END 5 */
}

void STM32CubeAI_Studio_AI_Process(void)
{
    main_loop();
} 

void STM32CubeAI_Studio_AI_Deinit(void)
{
    aiDeinit();
} 


#ifdef __cplusplus
}
#endif
