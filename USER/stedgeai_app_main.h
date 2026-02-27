#ifndef __STEDGEAI_APP_MAIN_H
#define __STEDGEAI_APP_MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ai_platform.h"
#include "network.h"
#include "network_data.h"

#define STAI_FORMAT int8_t

typedef struct
{
    ai_i8 midx[STAI_NETWORK_OUT_1_CHANNEL];
    ai_i8 midy[STAI_NETWORK_OUT_1_CHANNEL];
    ai_i8 w[STAI_NETWORK_OUT_1_CHANNEL];
    ai_i8 h[STAI_NETWORK_OUT_1_CHANNEL];
    ai_i8 object[STAI_NETWORK_OUT_1_HEIGHT - 4][STAI_NETWORK_OUT_1_CHANNEL];
}CHANNEL;

int aiInit(void);
stai_return_code aiRun(void);

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

#endif  // __STEDGEAI_APP_MAIN_H
