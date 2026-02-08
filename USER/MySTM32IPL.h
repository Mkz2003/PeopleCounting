#ifndef __MYSTM32IPL_H
#define __MYSTM32IPL_H

#include "stm32ipl.h"

#define MIDIMG_WIDTH   (Display_Width / 2)
#define MIDIMG_HEIGHT  (Display_Height / 2)

typedef enum
{
    FACE = 0,
    EYE,
}DetectionType;

typedef enum
{
    NONE = 0,
    Flip,
    Mirror,
    FlipMirror,
    Rotation90,
    Rotation180,
    Rotation270,
}ReplaceType;

void STM32IPL_MemInit(void);
void CascadeDetection(image_t* srcImg, array_t* _array, DetectionType detection, ReplaceType replace);

#endif /* __MYSTM32IPL_H */
