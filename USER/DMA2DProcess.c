#include "DMA2DProcess.h"

#include "main.h"

/**
 * @brief Blend colors using DMA2D.
 * @param pForeground: Pointer to the foreground image data.
 * @param pBackground: Pointer to the background image data.
 * @param pDestination: Pointer to the destination image data.
 * @param offsetForeground: Foreground image line offset.
 * @param offsetBackground: Background image line offset.
 * @param offsetDestination: Destination image line offset.
 * @param width: Width of the image in pixels.
 * @param height: Height of the image in pixels.
 * @param foregroundPixelFormat: Foreground Pixel format (e.g., DMA2D_OUTPUT_RGB565).
 * @param backgroundPixelFormat: Background Pixel format (e.g., DMA2D_OUTPUT_RGB565).
 * @param destinationPixelFormat: Destination Pixel format (e.g., DMA2D_OUTPUT_RGB565).
 * @param opacity: Opacity of the foreground image (0-255).
 * @return none
 */
void DMA2D_BlendColors(void* pForeground, void* pBackground, void* pDestination,
        uint32_t offsetForeground, uint32_t offsetBackground, uint32_t offsetDestination,
        uint16_t width, uint16_t height,
        uint32_t foregroundPixelFormat, uint32_t backgroundPixelFormat, uint32_t destinationPixelFormat, uint8_t opacity) {

    /* 等待DMA2D传输完成 */
    while (DMA2D->CR & DMA2D_CR_START);

    DMA2D->CR    = DMA2D_M2M_BLEND;             // 设置工作模式为存储器到存储器并带颜色混合

    DMA2D->FGMAR = (uint32_t)pForeground;               // 设置前景数据内存地址
    DMA2D->BGMAR = (uint32_t)pBackground;               // 设置背景数据内存地址
    DMA2D->OMAR  = (uint32_t)pDestination;              // 设置数据输出内存地址

    DMA2D->FGOR  = offsetForeground;                   // 设置前景数据传输偏移
    DMA2D->BGOR  = offsetBackground;                   // 设置背景数据传输偏移
    DMA2D->OOR   = offsetDestination;                 // 设置数据输出传输偏移

    DMA2D->NLR = (uint32_t)(width << 16) | (uint16_t)height; // 设置图像数据宽高（像素）
    
    DMA2D->FGPFCCR = foregroundPixelFormat                // 设置前景色颜色格式
            | (1UL << 16)                       // 忽略前景颜色数据中的Alpha通道
            | ((uint32_t)opacity << 24);            // 设置前景色不透明度

    DMA2D->BGPFCCR = backgroundPixelFormat;               // 设置背景颜色格式
    DMA2D->OPFCCR  = destinationPixelFormat;                // 设置输出颜色格式

    /* 启动传输 */
    DMA2D->CR   |= DMA2D_CR_START;

    /* 等待DMA2D传输完成 */
    while (DMA2D->CR & DMA2D_CR_START);
}

/**
  * @brief  DMA2D格式转换功能
  * @param  pSrc: 源图像地址
  * @param  pDst: 目标图像地址
  * @param  width: 图像宽度（像素数）
  * @param  height: 图像高度（像素数）
  * @param  offsetSrc: 源图像行偏移（字节数）
  * @param  offsetDst: 目标图像行偏移（字节数）
  * @param  sourcePixelFormat: 源像素格式
  * @param  destPixelFormat: 目标像素格式
  * @retval None
  */
void DMA2D_FormatConvert(void *pSrc, void *pDst, int width, int height, 
                         int offsetSrc, int offsetDst, 
                         uint32_t sourcePixelFormat, uint32_t destPixelFormat)
{
    /* 等待DMA2D传输完成 */
    while (DMA2D->CR & DMA2D_CR_START);

    /* DMA2D配置为格式转换模式（存储器到存储器并带像素格式转换）*/
    DMA2D->CR      = DMA2D_M2M_PFC;  /* 模式：存储器到存储器并带像素格式转换 */
    DMA2D->FGMAR   = (uint32_t)pSrc;                     /* 前景存储器地址（源） */
    DMA2D->OMAR    = (uint32_t)pDst;                     /* 输出存储器地址（目标） */
    DMA2D->FGOR    = offsetSrc;                         /* 前景偏移 */
    DMA2D->OOR     = offsetDst;                         /* 输出偏移 */
    DMA2D->FGPFCCR = sourcePixelFormat;                  /* 前景像素格式控制 */
    DMA2D->OPFCCR  = destPixelFormat;                    /* 输出像素格式控制 */
    DMA2D->NLR     = (uint32_t)(width << 16) | (uint16_t)height; /* 行数和每行像素数 */

    /* 启动传输 */
    DMA2D->CR   |= DMA2D_CR_START;

    /* 等待DMA2D传输完成 */
    while (DMA2D->CR & DMA2D_CR_START);
}

/**
 * @brief Copy memory using DMA2D.
 * @param pSrc: Pointer to the source image data.
 * @param pDst: Pointer to the destination image data.
 * @param width: Width of the image in pixels.
 * @param height: Height of the image in pixels.
 * @param offsetSrc: Source image line offset.
 * @param offseteDst: Destination image line offset.
 * @param pixelFormat: Pixel format (e.g., DMA2D_OUTPUT_RGB565).
 * @return none
 */
void DMA2D_MemCopy(void * pSrc, void * pDst, int width, int height, int offsetSrc, int offseteDst, uint32_t pixelFormat)
{
    /* 等待DMA2D传输完成 */
    while (DMA2D->CR & DMA2D_CR_START);

    /* DMA2D配置 */
    DMA2D->CR      = DMA2D_M2M;
    DMA2D->FGMAR   = (uint32_t)pSrc;
    DMA2D->OMAR    = (uint32_t)pDst;
    DMA2D->FGOR    = offsetSrc;
    DMA2D->OOR     = offseteDst;
    DMA2D->FGPFCCR = pixelFormat;
    DMA2D->NLR     = (uint32_t)(width << 16) | (uint16_t)height;

    /* 启动传输 */
    DMA2D->CR   |= DMA2D_CR_START;

    /* 等待DMA2D传输完成 */
    while (DMA2D->CR & DMA2D_CR_START);
}

/**
 * @brief Fill a rectangular area with a fixed color using DMA2D (R2M mode).
 * @param pDst: Pointer to the destination image data.
 * @param width: Width of the area in pixels.
 * @param height: Height of the area in pixels.
 * @param offsetDst: Destination line offset (in pixels).
 * @param pixelFormat: Output pixel format (e.g., DMA2D_OUTPUT_RGB565).
 * @param color: Color value to fill, formatted according to pixelFormat.
 * @return none
 */
void DMA2D_Fill(void * pDst, int width, int height, int offsetDst, uint32_t pixelFormat, uint32_t color)
{
    /* Wait for DMA2D to be ready */
    while (DMA2D->CR & DMA2D_CR_START);

    /* Configure DMA2D in Register-to-Memory mode */
    DMA2D->CR      = DMA2D_R2M;                /* R2M mode */
    DMA2D->OMAR    = (uint32_t)pDst;            /* Destination address */
    DMA2D->OOR     = offsetDst;                  /* Output line offset (pixels) */
    DMA2D->OPFCCR  = pixelFormat;                 /* Output pixel format */
    DMA2D->OCOLR   = color;                        /* Fill color */
    DMA2D->NLR     = (uint32_t)(width << 16) | (uint16_t)height; /* Number of pixels per line and lines */

    /* Start the transfer */
    DMA2D->CR |= DMA2D_CR_START;

    /* Wait for transfer completion */
    while (DMA2D->CR & DMA2D_CR_START);
}
