/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "MyCode.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

TickType_t DetectObject_Tick_Start = 0, DetectObject_Tick_End = 0;         	// 用于保存当前的时间计数值
float DetectObject_FPS = 0.0f;
array_t* faces = NULL,  * eyes = NULL, * faceIDs = NULL;

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for LCDRefresh */
osThreadId_t LCDRefreshHandle;
const osThreadAttr_t LCDRefresh_attributes = {
  .name = "LCDRefresh",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for LEDToggle */
osThreadId_t LEDToggleHandle;
const osThreadAttr_t LEDToggle_attributes = {
  .name = "LEDToggle",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for ImageProcess */
osThreadId_t ImageProcessHandle;
const osThreadAttr_t ImageProcess_attributes = {
  .name = "ImageProcess",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for ObjectDetect */
osThreadId_t ObjectDetectHandle;
const osThreadAttr_t ObjectDetect_attributes = {
  .name = "ObjectDetect",
  .stack_size = 2048 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void LCDRefreshTask(void *argument);
void LEDToggleTask(void *argument);
void ImageProcessTask(void *argument);
void ObjectDetectTask(void *argument);

extern void MX_USB_DEVICE_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void configureTimerForRunTimeStats(void);
unsigned long getRunTimeCounterValue(void);

/* USER CODE BEGIN 1 */
/* Functions needed when configGENERATE_RUN_TIME_STATS is on */
__weak void configureTimerForRunTimeStats(void)
{

}

__weak unsigned long getRunTimeCounterValue(void)
{
return 0;
}
/* USER CODE END 1 */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of LCDRefresh */
  LCDRefreshHandle = osThreadNew(LCDRefreshTask, NULL, &LCDRefresh_attributes);

  /* creation of LEDToggle */
  LEDToggleHandle = osThreadNew(LEDToggleTask, NULL, &LEDToggle_attributes);

  /* creation of ImageProcess */
  ImageProcessHandle = osThreadNew(ImageProcessTask, NULL, &ImageProcess_attributes);

  /* creation of ObjectDetect */
  ObjectDetectHandle = osThreadNew(ObjectDetectTask, NULL, &ObjectDetect_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* init code for USB_DEVICE */
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_LCDRefreshTask */
/**
* @brief Function implementing the LCDRefresh thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_LCDRefreshTask */
void LCDRefreshTask(void *argument)
{
  /* USER CODE BEGIN LCDRefreshTask */
  extern size_t umm_free_heap_size( void );
  extern size_t umm_max_free_block_size( void );
  extern int umm_usage_metric( void );
  extern int umm_fragmentation_metric( void );

  extern uint16_t* pCamera_Image;
  extern __IO float OV5640_FPS ;      // 帧率
  extern uint16_t LCD_Buff[LCD_Height][LCD_Width];
  uint16_t thickness = 2;		// Set the thickness of the rectangle (pixels).
	bool fill = false;			// Avoid to fill the rectangle.

  __attribute__((section(".ram_d3_bss"))) __IO static uint16_t adcval[3] = {0};

  HAL_ADCEx_Calibration_Start(&hadc3, ADC_CALIB_OFFSET_LINEARITY, ADC_SINGLE_ENDED);
  HAL_ADC_Start_DMA(&hadc3, (uint32_t*)adcval, 3);

  float vrefint_cal_vref = (VREFINT_CAL_VREF / 1000.0f);
  float temperature_cal1_temp = TEMPSENSOR_CAL1_TEMP;
  float temperature_cal2_temp = TEMPSENSOR_CAL2_TEMP;
  float vrefint_cal_addr = (*VREFINT_CAL_ADDR);
  float tempsensor_cal1_addr = (*TEMPSENSOR_CAL1_ADDR);
  float tempsensor_cal2_addr = (*TEMPSENSOR_CAL2_ADDR);

  SPI_LCD_Init();

  /* Infinite loop */
  for(;;)
  {
    // 计算LCD刷新帧率
    static TickType_t LCD_Tick_Start = 0, LCD_Tick_End = 0;
    float LCD_Tick_FPS; 
    LCD_Tick_End = xTaskGetTickCount();
    LCD_Tick_FPS = 1000.0f / (LCD_Tick_End - LCD_Tick_Start);
    LCD_Tick_Start = LCD_Tick_End;

    // sprintf(s, "ADC1: %d, ADC2: %d, ADC3: %d\n", adcval[0], adcval[1], adcval[2]);
    // CDC_Transmit_FS((uint8_t*)s, strlen(s));
    float Vref = vrefint_cal_vref / vrefint_cal_addr * adcval[2];
    float Vbat = adcval[0] * Vref / (float)(1 << 16) * 4.0f;
    float Temp = temperature_cal1_temp
                  + (adcval[1] * vrefint_cal_vref / Vref - tempsensor_cal1_addr)
                    * (temperature_cal2_temp - temperature_cal1_temp)
                    / (tempsensor_cal2_addr - tempsensor_cal1_addr);

    image_t srcImg, dstImg;
    STM32Ipl_Init(&srcImg, Display_Width, Display_Height, IMAGE_BPP_RGB565, (uint8_t*)pCamera_Image);
    STM32Ipl_Init(&dstImg, LCD_Width, LCD_Height, IMAGE_BPP_RGB565, (uint8_t*)LCD_Buff);
    STM32Ipl_Resize(&srcImg, &dstImg, RESIZE_NEAREST);  // 将图像缩放到LCD分辨率，等价于LCD_ShowImage

    for (int i = 0; i < array_length(faceIDs); i++) // 一定要实时读faceIDs的状态，防止其他任务更改faceIDs的状态导致未定义行为
    {
        faceIDObject_t *f = array_at(faceIDs, i);
        // Draw the bounding box around each detected face.
        uint16_t x = f->r.x * LCD_Height - (LCD_Height - LCD_Width) / 2;
        uint16_t y = f->r.y * LCD_Height;
        uint16_t w = f->r.width * LCD_Height;
        uint16_t h = f->r.height * LCD_Height;
        stm32ipl_color_t color = f->color;
        STM32Ipl_DrawRectangle(&dstImg, x, y, w, h, color, thickness, fill);
        LCD_printf(x / 12, (LCD_Height - (y + h)) / 6, "%d", f->id);
        LCD_printf((x + w) / 12, (LCD_Height - (y + h)) / 6, "%.2f", f->score);
    }   
    // LCD_Clear();
    // LCD_ShowImage(0, 0, (uint16_t *)LCDBackImage);  // 将图像数据复制到屏幕
    // LCD_ShowImage(0, 0, (uint16_t *)pCamera_Image);  // 将图像数据复制到屏幕
    LCD_printf(15, 1, "Screen: %.3f, Camera: %.3f, OD: %.3f", LCD_Tick_FPS, OV5640_FPS, DetectObject_FPS);
    LCD_printf(16, 1, "Time: %d:%02d:%02d:%02d:%03d", LCD_Tick_End / 86400000, LCD_Tick_End / 3600000 % 24, LCD_Tick_End / 60000 % 60, LCD_Tick_End / 1000 % 60, LCD_Tick_End % 1000);
    LCD_printf(17, 1, "Vbat: %.3fV, Temp: %.3f^C, Vref: %.3fV", Vbat, Temp, Vref);
    LCD_printf(18, 1, "Face: %d", array_length(faceIDs));
    LCD_printf(19, 1, "%d, %d, %d, %d", umm_free_heap_size(), umm_max_free_block_size(), umm_usage_metric(), umm_fragmentation_metric());
    LCD_SPI_Transmit();

    // uint32_t dstSize = JPEGEncode((uint8_t*)LCD_Buff, LCD_Height * LCD_Width * 2);
    // CDC_Transmit_FS(jpeg_buffer, dstSize);

    osDelayUntil(LCD_Tick_End + 1000 / 10); // ~10fps
  }
  /* USER CODE END LCDRefreshTask */
}

/* USER CODE BEGIN Header_LEDToggleTask */
/**
* @brief Function implementing the LEDToggle thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_LEDToggleTask */
void LEDToggleTask(void *argument)
{
  /* USER CODE BEGIN LEDToggleTask */
  /* Infinite loop */
  for(;;)
  {
    HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
    osDelayUntil(xTaskGetTickCount() / 1000 * 1000 + 1000);
  }
  /* USER CODE END LEDToggleTask */
}

/* USER CODE BEGIN Header_ImageProcessTask */
/**
* @brief Function implementing the ImageProcess thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_ImageProcessTask */
void ImageProcessTask(void *argument)
{
  /* USER CODE BEGIN ImageProcessTask */

  extern OV5640_FrameState ov5640_FrameState;
  extern uint16_t* pCamera_Image; // 摄像头数据缓冲区指针

  JPEGEncodeInit();

	DCMI_OV5640_Init();   			 	// DCMI以及OV5640初始化
	OV5640_AF_Download_Firmware();	// 写入自动对焦固件
	OV5640_AF_Trigger_Constant();		// 自动对焦 ，持续 触发，当OV5640检测到当前画面不在焦点时，会一直触发对焦
  OV5640_StartImageCapture();

  /* Infinite loop */
  for(;;)
  {
    if(ov5640_FrameState.imageProcessState)  // 等待一帧数据传输完成
    {
      ov5640_FrameState.imageProcessState = 0;
      // uint32_t dstSize = JPEGEncode((uint8_t*)pCamera_Image, Display_Height * Display_Width * 2);
      // CDC_Transmit_FS(jpeg_buffer, dstSize);
    }
    else
    {
      osThreadYield();
    }
  }
  /* USER CODE END ImageProcessTask */
}

/* USER CODE BEGIN Header_ObjectDetectTask */
/**
* @brief Function implementing the ObjectDetect thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_ObjectDetectTask */
void ObjectDetectTask(void *argument)
{
  /* USER CODE BEGIN ObjectDetectTask */

  extern OV5640_FrameState ov5640_FrameState;
  extern uint16_t* pCamera_Image; // 摄像头数据缓冲区指针
  extern OV5640_FrameState ov5640_FrameState;
  extern void xfree(void *mem);

  STM32IPL_MemInit();
  array_alloc(&faces, xfree);
  array_alloc(&eyes, xfree);
  array_alloc(&faceIDs, xfree);

  extern stai_ptr m_inputs[STAI_NETWORK_IN_NUM];
  extern stai_ptr m_outputs[STAI_NETWORK_OUT_NUM];

  #define ResizeHeight  (STAI_NETWORK_IN_1_WIDTH)
  #define ResizeWidth   (Display_Width * STAI_NETWORK_IN_1_HEIGHT / Display_Height)

  __attribute__((section(".sdram_bss"), aligned(32))) static uint16_t resizedImage[ResizeHeight][ResizeWidth];
  // __attribute__((section(".sdram_bss"), aligned(32))) static uint16_t resizedImage[STAI_NETWORK_IN_1_WIDTH][STAI_NETWORK_IN_1_HEIGHT];

  aiInit();

  CHANNEL* outData = (CHANNEL*)m_outputs[0];

  /* Infinite loop */
  for(;;)
  {
    if(ov5640_FrameState.objectDetectState)  // 等待一帧数据传输完成
    {
      ov5640_FrameState.objectDetectState = 0;

      // 计算帧率
      DetectObject_Tick_End = xTaskGetTickCount();
			DetectObject_FPS = 1000.0f / (DetectObject_Tick_End - DetectObject_Tick_Start);
			DetectObject_Tick_Start = DetectObject_Tick_End;

      // 这里可以对图像数据进行处理，例如人脸识别、目标检测等
      // 将摄像头数据指向srcImg，进行目标检测
      image_t srcImg, resizedImg;
      STM32Ipl_Init(&srcImg, Display_Width, Display_Height, IMAGE_BPP_RGB565, (uint8_t*)pCamera_Image);

      STM32Ipl_Init(&resizedImg, ResizeWidth, ResizeHeight, IMAGE_BPP_RGB565, resizedImage);
      // STM32Ipl_Init(&resizedImg, STAI_NETWORK_IN_1_HEIGHT, STAI_NETWORK_IN_1_WIDTH, IMAGE_BPP_RGB565, resizedImage);
      
      STM32Ipl_Resize(&srcImg, &resizedImg, RESIZE_NEAREST);
      STM32Ipl_Rotation90(&resizedImg, &resizedImg);

      const size_t offset = (sizeof(STAI_FORMAT) * 3) * STAI_NETWORK_IN_1_WIDTH * (STAI_NETWORK_IN_1_HEIGHT - ResizeWidth) / 2;
      // const size_t offset = 0;
      DMA2D_Fill((uint8_t*)m_inputs[0], 
                  STAI_NETWORK_IN_1_WIDTH, 
                  STAI_NETWORK_IN_1_HEIGHT, 
                  0,
                  DMA2D_OUTPUT_RGB888, 
                  0x00111111
                );
      DMA2D_FormatConvert(resizedImage, 
                            (uint8_t*)m_inputs[0] + offset, 
                            resizedImg.w, 
                            resizedImg.h, 
                            0, 
                            0, 
                            DMA2D_INPUT_RGB565, 
                            DMA2D_OUTPUT_RGB888
                          );

      // 转换图像为int8
      // for(size_t i = 0; i < ResizeHeight * ResizeWidth * 3; i++) *((ai_i8*)m_inputs[0] + offset + i) = rgb888_To_ai_i8(*((uint8_t*)m_inputs[0] + offset + i));
      for(size_t i = 0; i < ResizeHeight * ResizeWidth * 3; i++) *((int8_t*)m_inputs[0] + offset + i) = ai_float_32_To_ai_i8(*((uint8_t*)m_inputs[0] + offset + i) / 255.0f);
                        
      uint32_t a = aiRun();
      
      if(a == 0)
      {
        array_clear(faces);
        int32_t label = 0;
        for(int i = 0; i < STAI_NETWORK_OUT_1_CHANNEL; i++)
        {
          // 找认定label类为最大概率的检测框
          uint8_t flag = 0;
          for(int j = 0; j < STAI_NETWORK_OUT_1_HEIGHT - 4; j++)
          {
            if(j != label && ai_i8_To_ai_float(outData->object[label][i]) <= ai_i8_To_ai_float(outData->object[j][i]))
            {
              flag = 1;
              break;
            }
          }
          if(flag || ai_i8_To_ai_float(outData->object[label][i]) <= 0.1f) continue;

          // 存入归一化数据
          faceObject_t* f = (faceObject_t*)xalloc(sizeof(faceObject_t));

          f->rect.x = (ai_i8_To_ai_float(outData->midy[i]) - ai_i8_To_ai_float(outData->h[i]) / 2);
          f->rect.y = (1.0f - ai_i8_To_ai_float(outData->midx[i]) - ai_i8_To_ai_float(outData->w[i]) / 2);
          f->rect.width =  ai_i8_To_ai_float(outData->h[i]);
          f->rect.height = ai_i8_To_ai_float(outData->w[i]);
          f->label = label;
          f->prob = ai_i8_To_ai_float(outData->object[label][i]);

          array_push_back(faces, f);
        }
      }

      get(faces, faceIDs);

      // if(array_length(faceIDs) > 0) CDC_printf("%ds\n", xTaskGetTickCount() / 1000);
      // for(int i = 0; i < array_length(faceIDs); i++)
      // {
      //   faceIDObject_t *f = array_at(faceIDs, i);
      //   CDC_printf("id: %d, x: %d, y: %d, w: %d, h: %d\n", f->id, f->r.x, f->r.y, f->r.w, f->r.h);
      // }
      // if(array_length(faceIDs) > 0) CDC_printf("\n");

    }
    else
    {
      osThreadYield();
    }
  }
  /* USER CODE END ObjectDetectTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

