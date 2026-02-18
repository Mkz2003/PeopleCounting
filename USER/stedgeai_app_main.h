#ifndef __STEDGEAI_APP_MAIN_H
#define __STEDGEAI_APP_MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "network.h"
#include "network_data.h"

#define STAI_FORMAT int8_t

typedef struct
{
    int8_t x[STAI_NETWORK_OUT_1_CHANNEL];
    int8_t y[STAI_NETWORK_OUT_1_CHANNEL];
    int8_t w[STAI_NETWORK_OUT_1_CHANNEL];
    int8_t h[STAI_NETWORK_OUT_1_CHANNEL];
    int8_t object[STAI_NETWORK_OUT_1_HEIGHT - 4][STAI_NETWORK_OUT_1_CHANNEL];
}CHANNEL;


int aiInit(void);
stai_return_code aiRun(void);

static inline float int8Tofloat32(int8_t x)
{
    return (uint8_t)(x - STAI_NETWORK_OUT_1_ZERO_POINT) * STAI_NETWORK_OUT_1_SCALE;
}

static inline int8_t float32Toint8(float32_t x)
{
    return (int8_t)(x / STAI_NETWORK_IN_1_SCALE - STAI_NETWORK_IN_1_ZERO_POINT);
}

#endif  // __STEDGEAI_APP_MAIN_H
