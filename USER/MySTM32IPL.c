#include "MySTM32IPL.h"

#include "array.h"
#include "stm32ipl.h"
#include "stm32ipl_imlib_int.h"


__attribute__((section(".sdram_bss"), aligned(32))) static uint8_t stm32iplBuffer[STM32IPL_INT_BUFFER_SIZE]; // STM32 IPL库使用的内部缓冲区
static cascade_t cascade;			// The cascade structure used by the object detector.

void STM32IPL_Init(void)
{
    STM32Ipl_InitLib(stm32iplBuffer, STM32IPL_INT_BUFFER_SIZE);
    STM32Ipl_LoadFaceCascade(&cascade);
}

/**
 * @brief Detects faces in the source image and draws bounding boxes with black background.
 * @param srcImg RGB565 source image
 * @param _faces array with coordinates
 * @return none
 */
void FaceDetection(image_t* srcImg, array_t* _faces)
{
    __attribute__((section(".sdram_bss"), aligned(32))) static uint16_t midImageRGB[MIDIMG_HEIGHT][MIDIMG_WIDTH];
    __attribute__((section(".sdram_bss"), aligned(32))) static uint8_t midImageGray[MIDIMG_HEIGHT][MIDIMG_WIDTH];

	float scaleFactor = 1.25f;	// Modify this value to detect objects at 
								// different scale (must be > 1.0f).
	float threshold = 0.75f;	// Modify this value to tune the detection rate against
    							// the false positive rate (0.0f - 1.0f).
    image_t _midImgRGB, _midImgGray;
    image_t* midImgRGB = &_midImgRGB, *midImgGray = &_midImgGray;
    STM32Ipl_Init(midImgRGB, MIDIMG_WIDTH, MIDIMG_HEIGHT, IMAGE_BPP_RGB565, (uint8_t*)midImageRGB);
    STM32Ipl_Init(midImgGray, MIDIMG_WIDTH, MIDIMG_HEIGHT, IMAGE_BPP_GRAYSCALE, (uint8_t*)midImageGray);

    STM32Ipl_Resize(srcImg, midImgRGB, RESIZE_NEAREST);  // 将图像缩放到ai分辨率
    // STM32Ipl_Gaussian(midImgRGB, 1, true, true, NULL);
    STM32Ipl_Convert(midImgRGB, midImgGray);

    array_t *faces = 0;
    uint32_t faceCount = 0;
    // Detect faces. No ROI is passed, so the full image is analyzed.
    if (stm32ipl_err_Ok == STM32Ipl_DetectObject(midImgGray, &faces, NULL, &cascade, scaleFactor, threshold))
    {
        array_clear(_faces);
        // Get the number of detected faces.
        faceCount = array_length(faces);
        // Get the bounding box for each detected face.
        for (int i = 0; i < faceCount; i++)
        {
            rectangle_t *r = array_at(faces, i);
            // Draw the bounding box around each detected face.
            array_push_back(_faces, rectangle_alloc(r->x, r->y, r->w, r->h));
        }

        // Release the array containing the faces.
        array_free(faces);
        faces = NULL;
    }
}



