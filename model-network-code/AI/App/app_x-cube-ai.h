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
    ai_i8 midx[STAI_NETWORK_OUT_1_CHANNEL];
    ai_i8 midy[STAI_NETWORK_OUT_1_CHANNEL];
    ai_i8 w[STAI_NETWORK_OUT_1_CHANNEL];
    ai_i8 h[STAI_NETWORK_OUT_1_CHANNEL];
    ai_i8 object[STAI_NETWORK_OUT_1_HEIGHT - 4][STAI_NETWORK_OUT_1_CHANNEL];
}CHANNEL;

static inline ai_float ai_i8_To_ai_float(ai_i8 x)
{
    static float table[1 << sizeof(ai_i8) * 8];
    static bool init = false;
    if(init == false)
    {
        ai_i8 ai_i8_max = 127, ai_i8_min = -128;
        for(int i = ai_i8_min; i <= ai_i8_max; i++) table[i - STAI_NETWORK_OUT_1_ZERO_POINT] = (i - STAI_NETWORK_OUT_1_ZERO_POINT) * STAI_NETWORK_OUT_1_SCALE;
        init = true;
    }
    return table[x - STAI_NETWORK_OUT_1_ZERO_POINT];
}

static inline ai_i8 ai_float_32_To_ai_i8(ai_float x)
{
    return (ai_i8)(x / STAI_NETWORK_IN_1_SCALE + STAI_NETWORK_IN_1_ZERO_POINT);
}

static inline ai_i8 rgb888_To_ai_i8(uint8_t x)
{
    return (ai_i8)(x + STAI_NETWORK_IN_1_ZERO_POINT);
}

/* USER CODE END includes */


#ifdef __cplusplus
}
#endif
#endif /*__STMicroelectronics_ST_EDGE_AI_4.0.0-20500 359356bb0_H */
