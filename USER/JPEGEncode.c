#include "JPEGEncode.h"

#include "cmsis_os2.h"
#include "MyCode.h"

#define YCBCR_420_BLOCK_SIZE       384     /* YCbCr 4:2:0 MCU : 4 8x8 blocks of Y + 1 8x8 block of Cb + 1 8x8 block of Cr   */
#define YCBCR_422_BLOCK_SIZE       256     /* YCbCr 4:2:2 MCU : 2 8x8 blocks of Y + 1 8x8 block of Cb + 1 8x8 block of Cr   */
#define YCBCR_444_BLOCK_SIZE       192     /* YCbCr 4:4:4 MCU : 1 8x8 block of Y + 1 8x8 block of Cb + 1 8x8 block of Cr   */

#define GRAY_444_BLOCK_SIZE        64      /* GrayScale MCU : 1 8x8 block of Y */

#define CMYK_444_BLOCK_SIZE        256     /* CMYK MCU : 1 8x8 blocks of Cyan + 1 8x8 block Magenta + 1 8x8 block of Yellow and 1 8x8 block of BlacK */

#define BLOCK_SIZE YCBCR_422_BLOCK_SIZE
#define CHUNC_SINGLE_MCU_CONVERTION 16

JPEG_ConfTypeDef jpegConf = {0};

JPEG_RGBToYCbCr_Convert_Function jpeg_RGBToYCbCr_Convert_Function;
uint32_t imageNbMCUs;
__IO uint32_t numberMCU;
__IO uint32_t mcuCount;
__IO uint32_t length;
__IO uint8_t Jpeg_HWEncodingEnd;

extern JPEG_HandleTypeDef hjpeg;

__attribute__((section(".sdram_bss"), aligned(32))) uint32_t MCU[Display_Width * Display_Height];
__attribute__((section(".sdram_bss"), aligned(32))) uint8_t jpeg_buffer[64 * 1024 - 1];

void JPEGEncodeInit()
{
    // 初始化JPEG编码参数
    jpegConf.ColorSpace = JPEG_YCBCR_COLORSPACE;
    jpegConf.ChromaSubsampling = JPEG_422_SUBSAMPLING;
    jpegConf.ImageWidth = Display_Width;
    jpegConf.ImageHeight = Display_Height;
    jpegConf.ImageQuality = 50;
    // 应用配置
    if (HAL_JPEG_ConfigEncoding(&hjpeg, &jpegConf) != HAL_OK)
    {
        Error_Handler();
    }

    JPEG_InitColorTables();
    JPEG_GetEncodeColorConvertFunc(&jpegConf, &jpeg_RGBToYCbCr_Convert_Function, &imageNbMCUs);
}

uint32_t JPEGEncode(uint8_t* srcImage, uint32_t srcSize)
{   
    Jpeg_HWEncodingEnd = 0;
    length = 0;
    mcuCount = 0;
    uint32_t ConvertedDataCount;
    numberMCU = jpeg_RGBToYCbCr_Convert_Function(srcImage, (uint8_t*)MCU, 0, srcSize, &ConvertedDataCount);
    mcuCount += CHUNC_SINGLE_MCU_CONVERTION;    
    HAL_JPEG_Encode_DMA(&hjpeg, (uint8_t*)MCU, CHUNC_SINGLE_MCU_CONVERTION * BLOCK_SIZE, jpeg_buffer, sizeof(jpeg_buffer));
    while(Jpeg_HWEncodingEnd != 1)
    {
        #ifdef CMSIS_OS2_H_
        osThreadYield();  // 让出CPU时间片
        #endif
    }
    return length;
}

void HAL_JPEG_DataReadyCallback(JPEG_HandleTypeDef *hjpeg, uint8_t *pDataOut, uint32_t OutDataLength)
{
    length += OutDataLength;
    HAL_JPEG_ConfigOutputBuffer(hjpeg, jpeg_buffer + length, sizeof(jpeg_buffer) - length);
}

void HAL_JPEG_GetDataCallback(JPEG_HandleTypeDef *hjpeg, uint32_t NbDecodedData)
{
    if(mcuCount < numberMCU)
    {
        if(mcuCount + CHUNC_SINGLE_MCU_CONVERTION <= numberMCU)
        {
            HAL_JPEG_ConfigInputBuffer(hjpeg, (uint8_t*)MCU + mcuCount * BLOCK_SIZE, CHUNC_SINGLE_MCU_CONVERTION * BLOCK_SIZE);
            mcuCount += CHUNC_SINGLE_MCU_CONVERTION;
        }
        else
        {
            HAL_JPEG_ConfigInputBuffer(hjpeg, (uint8_t*)MCU + mcuCount * BLOCK_SIZE, (numberMCU - mcuCount) * BLOCK_SIZE);
            mcuCount += numberMCU - mcuCount;
        }      
    }
}

void HAL_JPEG_EncodeCpltCallback(JPEG_HandleTypeDef *hjpeg)
{    
    Jpeg_HWEncodingEnd = 1; 
}

void HAL_JPEG_ErrorCallback(JPEG_HandleTypeDef *hjpeg)
{
    length = 0;
    Jpeg_HWEncodingEnd = 1; 
}
