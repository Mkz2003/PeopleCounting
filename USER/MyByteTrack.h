#ifndef __MYBYTETRACK_H
#define __MYBYTETRACK_H

#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus


#include "stm32ipl_imlib.h"

void get(array_t* inArray, array_t* outArray);

typedef struct
{
    rectangle_t r;
    int id;
    uint32_t color;
}faceIDObject_t;





#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // __MYBYTETRACK_H
