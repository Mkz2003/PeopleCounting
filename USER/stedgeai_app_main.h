#ifndef __STEDGEAI_APP_MAIN_H
#define __STEDGEAI_APP_MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "network.h"
#include "network_data.h"

#define STAI_FORMAT float32_t

typedef struct
{
    struct
    {
        STAI_FORMAT x, y, w, h, object, class;
    } box[STAI_NETWORK_OUT_1_HEIGHT][STAI_NETWORK_OUT_1_WIDTH][STAI_NETWORK_OUT_1_CHANNEL / 6];
}CHANNEL;


int aiInit(void);
stai_return_code aiRun(void);




#endif  // __STEDGEAI_APP_MAIN_H
