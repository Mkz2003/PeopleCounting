#ifndef __DMA2DPROCESS_H
#define __DMA2DPROCESS_H

#include "main.h"

void DMA2D_BlendColors(void* pForeground, void* pBackground, void* pDestination,
        uint32_t offsetForeground, uint32_t offsetBackground, uint32_t offsetDestination,
        uint16_t width, uint16_t height,
        uint32_t pixelFormat, uint8_t opacity);

void DMA2D_MemCopy(void * pSrc, void * pDst, int width, int height, int offsetSrc, int offseteDst, uint32_t pixelFormat);

#endif /* __DMA2DPROCESS_H */