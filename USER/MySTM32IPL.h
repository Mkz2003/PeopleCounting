#ifndef __MYSTM32IPL_H
#define __MYSTM32IPL_H

#include "stm32ipl.h"

#define MIDIMG_WIDTH   240
#define MIDIMG_HEIGHT  320

void STM32IPL_Init(void);
void FaceDetection(image_t* srcImg, array_t* _faces);

#endif /* __MYSTM32IPL_H */