#ifndef __MYSTM32IPL_H
#define __MYSTM32IPL_H

#include "stm32ipl.h"

#define MIDIMG_WIDTH   Display_Width
#define MIDIMG_HEIGHT  Display_Height

void STM32IPL_Init(void);
void FaceDetection(image_t* srcImg, array_t* _faces);

#endif /* __MYSTM32IPL_H */