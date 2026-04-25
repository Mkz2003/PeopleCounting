#ifndef __DMA2DPROCESS_OS_H
#define __DMA2DPROCESS_OS_H

#include "main.h"

void DMA2D_BlendColors_OS(void* pForeground, void* pBackground, void* pDestination,
        uint32_t offsetForeground, uint32_t offsetBackground, uint32_t offsetDestination,
        uint16_t width, uint16_t height,
        uint32_t ForegroundpixelFormat, uint32_t BackgroundpixelFormat, uint32_t DestinationpixelFormat, uint8_t opacity);

void DMA2D_FormatConvert_OS(void *pSrc, void *pDst, int width, int height, 
                         int offsetSrc, int offsetDst, 
                         uint32_t sourcePixelFormat, uint32_t destPixelFormat, uint8_t swapRB);

void DMA2D_MemCopy_OS(void * pSrc, void * pDst, int width, int height, int offsetSrc, int offseteDst, uint32_t pixelFormat);

void DMA2D_Fill_OS(void * pDst, int width, int height, int offsetDst, uint32_t pixelFormat, uint32_t color);

#endif /* __DMA2DPROCESS_OS_H */
