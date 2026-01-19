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
 * @param pixelFormat: Pixel format (e.g., DMA2D_OUTPUT_RGB565).
 * @param opacity: Opacity of the foreground image (0-255).
 * @return none
 */
void DMA2D_BlendColors(void* pForeground, void* pBackground, void* pDestination,
        uint32_t offsetForeground, uint32_t offsetBackground, uint32_t offsetDestination,
        uint16_t width, uint16_t height,
        uint32_t pixelFormat, uint8_t opacity) {

    /* 等待DMA2D传输完成 */
    while (DMA2D->CR & DMA2D_CR_START);

    DMA2D->CR    = 0x00020000UL;             // 设置工作模式为存储器到存储器并带颜色混合

    DMA2D->FGMAR = (uint32_t)pForeground;               // 设置前景数据内存地址
    DMA2D->BGMAR = (uint32_t)pBackground;               // 设置背景数据内存地址
    DMA2D->OMAR  = (uint32_t)pDestination;              // 设置数据输出内存地址

    DMA2D->FGOR  = offsetForeground;                   // 设置前景数据传输偏移
    DMA2D->BGOR  = offsetBackground;                   // 设置背景数据传输偏移
    DMA2D->OOR   = offsetDestination;                 // 设置数据输出传输偏移

    DMA2D->NLR = (uint32_t)(width << 16) | (uint16_t)height; // 设置图像数据宽高（像素）
    
    DMA2D->FGPFCCR = pixelFormat                // 设置前景色颜色格式
            | (1UL << 16)                       // 忽略前景颜色数据中的Alpha通道
            | ((uint32_t)opacity << 24);            // 设置前景色不透明度

    DMA2D->BGPFCCR = pixelFormat;               // 设置背景颜色格式
    DMA2D->OPFCCR  = pixelFormat;                // 设置输出颜色格式

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
    DMA2D->CR      = 0x00000000UL;
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
