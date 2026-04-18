/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_AI_H
#define __APP_AI_H
#ifdef __cplusplus
extern "C" {
#endif
/**
  ******************************************************************************
  * @file    app_x-cube-ai.h
  * @author  X-CUBE-AI C code generator
  * @brief   AI entry function definitions
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
/* Includes ------------------------------------------------------------------*/
#include "stai.h"
#include "ai_datatypes_defines.h"

#include "network.h"

/* IO buffers ----------------------------------------------------------------*/

void STM32CubeAI_Studio_AI_Init(void);
void STM32CubeAI_Studio_AI_Process(void);
/* USER CODE BEGIN includes */

#define STAI_FORMAT ai_i8

typedef struct
{
    ai_float (*score)[STAI_NETWORK_OUT_1_HEIGHT][STAI_NETWORK_OUT_1_CHANNEL];
    struct
    {
        ai_float x1;
        ai_float y1;
        ai_float x2;
        ai_float y2;
    } (*code)[STAI_NETWORK_OUT_2_HEIGHT];
}CHANNEL;

/* USER CODE END includes */


#ifdef __cplusplus
}
#endif
#endif /*__STMicroelectronics_ST_EDGE_AI_4.0.0-20500 359356bb0_H */
