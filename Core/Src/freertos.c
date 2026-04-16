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
#include "FreeRTOS.h"
#include "cmsis_os2.h"

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

  __attribute__((section(".ram_d3_bss"))) __IO static struct {uint16_t Vbat, Temp, Vref;} adcval = {0};

  HAL_ADCEx_Calibration_Start(&hadc3, ADC_CALIB_OFFSET_LINEARITY, ADC_SINGLE_ENDED);
  HAL_ADC_Start_DMA(&hadc3, (uint32_t*)&adcval, sizeof(adcval) / sizeof(uint16_t));

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
    float Vref = vrefint_cal_vref / vrefint_cal_addr * adcval.Vref;
    float Vbat = adcval.Vbat * Vref / (float)(1 << 16) * 4.0f;
    float Temp = temperature_cal1_temp
                  + (adcval.Temp * vrefint_cal_vref / Vref - tempsensor_cal1_addr)
                    * (temperature_cal2_temp - temperature_cal1_temp)
                    / (tempsensor_cal2_addr - tempsensor_cal1_addr);

    image_t srcImg, dstImg;
    STM32Ipl_Init(&srcImg, Display_Width, Display_Height, IMAGE_BPP_RGB565, (uint8_t*)pCamera_Image);
    STM32Ipl_Init(&dstImg, LCD_Height, LCD_Width, IMAGE_BPP_RGB565, (uint8_t*)LCD_Buff);
    STM32Ipl_Rotation270(&dstImg, &srcImg);  // 将图像缩放到LCD分辨率，等价于LCD_ShowImage

    for (int i = 0; i < array_length(faceIDs); i++) // 一定要实时读faceIDs的状态，防止其他任务更改faceIDs的状态导致未定义行为
    {
        faceIDObject_t *f = array_at(faceIDs, i);
        // Draw the bounding box around each detected face.
        // uint16_t x = f->r.x * LCD_Height - (LCD_Height - LCD_Width) / 2;
        // uint16_t y = f->r.y * LCD_Height;
        // uint16_t w = f->r.width * LCD_Height;
        // uint16_t h = f->r.height * LCD_Height;
        uint16_t x = f->r.y * LCD_Height - (LCD_Height - LCD_Width) / 2;
        uint16_t y = (1.0f - f->r.x - f->r.width) * LCD_Height;
        uint16_t w = f->r.height * LCD_Height;
        uint16_t h = f->r.width * LCD_Height;
        stm32ipl_color_t color = f->color;
        STM32Ipl_DrawRectangle(&dstImg, x, y, w, h, color, thickness, fill);
        LCD_printf(x / 12, (LCD_Height - (y + h)) / 6, "%d", f->id);
        LCD_printf((x + w) / 12, (LCD_Height - (y + h)) / 6, "%.2f", f->score);
    }   
    // LCD_Clear();
    // LCD_ShowImage(0, 0, (uint16_t *)LCDBackImage);  // 将图像数据复制到屏幕
    // LCD_ShowImage(0, 0, (uint16_t *)pCamera_Image);  // 将图像数据复制到屏幕
    // LCD_printf(16, 1,"%d, %d, %d, %d", umm_free_heap_size(), umm_max_free_block_size(), umm_usage_metric(), umm_fragmentation_metric());
    // LCD_printf(7, 1, "Time: %d:%02d:%02d:%02d:%03d", LCD_Tick_End / 86400000, LCD_Tick_End / 3600000 % 24, LCD_Tick_End / 60000 % 60, LCD_Tick_End / 1000 % 60, LCD_Tick_End % 1000);
    LCD_printf(18, 1, "Vbat: %.3fV, Temp: %.3f^C, Vref: %.3fV", Vbat, Temp, Vref);
    LCD_printf(19, 1,  "Face: %d, Scr: %.3f, Cam: %.3f, OD: %.3f", array_length(faceIDs), LCD_Tick_FPS, OV5640_FPS, DetectObject_FPS);
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
      uint32_t dstSize = JPEGEncode((uint8_t*)pCamera_Image, Display_Height * Display_Width * 2);
      while(CDC_Transmit_FS(jpeg_buffer, dstSize) != USBD_OK) osThreadYield();
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
  extern void xfree(void *mem);

  STM32IPL_MemInit();
  array_alloc(&faces, xfree);
  array_alloc(&eyes, xfree);
  array_alloc(&faceIDs, xfree);

  STM32CubeAI_Studio_AI_Init();

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

      STM32CubeAI_Studio_AI_Process();

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

