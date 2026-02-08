#include "MySTM32IPL.h"

#include "array.h"
#include "lcd_spi_200.h"
#include "stm32ipl.h"
#include "stm32ipl_imlib_int.h"

__attribute__((section(".sdram_bss"), aligned(32))) static uint8_t stm32iplBuffer[STM32IPL_INT_BUFFER_SIZE]; // STM32 IPL库使用的内部缓冲区   
__attribute__((section(".sdram_bss"), aligned(32))) static uint16_t midImage[MIDIMG_HEIGHT][MIDIMG_WIDTH];
__attribute__((section(".sdram_bss"), aligned(32))) static uint16_t dstImage[MIDIMG_HEIGHT][MIDIMG_WIDTH];

cascade_t face_cascade;			// The cascade structure used by the object detector.
cascade_t eye_cascade;			// The cascade structure used by the object detector.

void STM32IPL_MemInit(void)
{
    STM32Ipl_InitLib(stm32iplBuffer, STM32IPL_INT_BUFFER_SIZE);
    STM32Ipl_LoadFaceCascade(&face_cascade);
    STM32Ipl_LoadEyeCascade(&eye_cascade);
}

/**
 * @brief Detects faces in the source image and draws bounding boxes with black background.
 * @param srcImg RGB565 source image
 * @param _faces array with coordinates. Its data will be refreshed with resultant data.
 * @param detection type of detection.
 * @return none
 */
void CascadeDetection(image_t* srcImg, array_t* _array, DetectionType detection, ReplaceType replace)
{
    image_t _midImg, * midImg = &_midImg;
    image_t _dstImg, * dstImg = &_dstImg;
    STM32Ipl_Init(midImg, MIDIMG_WIDTH, MIDIMG_HEIGHT, IMAGE_BPP_RGB565, (uint8_t*)midImage);
    STM32Ipl_Init(dstImg, MIDIMG_WIDTH, MIDIMG_HEIGHT, IMAGE_BPP_RGB565, (uint8_t*)dstImage);

    cascade_t* cascade;
    switch(detection)
    {
        case FACE: cascade = &face_cascade; break;
        case EYE:  cascade = &eye_cascade; break;
        default: cascade = &face_cascade; break;
    }

	float scaleFactor = 1.25f;	// Modify this value to detect objects at 
								// different scale (must be > 1.0f).
	float threshold = 0.75f;	// Modify this value to tune the detection rate against
    							// the false positive rate (0.0f - 1.0f).

    STM32Ipl_Resize(srcImg, midImg, RESIZE_NEAREST);  // 将图像缩放、旋转，同时将图像储存。
    switch(replace)
    {
        case Rotation270:
        {
            STM32Ipl_Rotation270(dstImg, midImg);
            break;
        }    
        default:
        {
            STM32Ipl_Copy(midImg, dstImg);
            break;
        }    
    }
    
    // STM32Ipl_Gaussian(midImg, 1, true, true, NULL);

    array_t *array = 0;
    uint32_t targetCount = 0;
    // Detect faces. No ROI is passed, so the full image is analyzed.
    if (stm32ipl_err_Ok == STM32Ipl_DetectObject(dstImg, &array, NULL, cascade, scaleFactor, threshold))
    {
        array_clear(_array);
        // Get the number of detected faces.
        targetCount = array_length(array);
        // Get the bounding box for each detected face.
        for (int i = 0; i < targetCount; i++)
        {
            rectangle_t *r = array_at(array, i);
            // Draw the bounding box around each detected face.
            switch(replace)
            {
                case Rotation270:
                {
                    array_push_back(_array, 
                                    rectangle_alloc((MIDIMG_WIDTH - r->y - r->h) * LCD_Width / MIDIMG_WIDTH, 
                                        r->x * LCD_Height / MIDIMG_HEIGHT, 
                                        r->h * LCD_Width / MIDIMG_WIDTH, 
                                        r->w * LCD_Height / MIDIMG_HEIGHT));    // transform coordinates because it is rotated 270.
                    break;
                }
                default:
                {
                    array_push_back(_array, rectangle_alloc(r->x * LCD_Width / MIDIMG_WIDTH, r->y * LCD_Height / MIDIMG_HEIGHT, r->w * LCD_Width / MIDIMG_WIDTH, r->h * LCD_Height / MIDIMG_HEIGHT));
                    break;
                }
            }
        }

        // Release the array containing the faces.
        array_free(array);
        array = NULL;
    }
}



