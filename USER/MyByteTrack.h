#ifndef __MYBYTETRACK_H
#define __MYBYTETRACK_H

#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus


#include "stm32ipl_imlib.h"
#include "AreaTest.h"

void get(array_t* inArray, array_t* outArray);

typedef struct
{
    struct {
		float x, y, width, height;
	} rect;
    int label;
    float prob;
}faceObject_t;

typedef struct
{
    struct {
		float x, y, width, height;
	} r;
    Point point;
    int id;
    uint32_t color;
    float score;
    int zone;
}faceIDObject_t;





#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // __MYBYTETRACK_H
