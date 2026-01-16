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

#include "adc.h"
#include "dcmi.h"
#include "dma.h"
#include "dma2d.h"
#include "ltdc.h"
#include "quadspi.h"
#include "rtc.h"
#include "spi.h"
#include "usart.h"
#include "usb_device.h"
#include "gpio.h"
#include "fmc.h"

#include "MyCode.h"
#include <stdio.h>
#include "usbd_cdc_if.h"
#include "lcd_spi_200.h"
#include "dcmi_ov5640.h"

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

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void LCDRefreshTask(void *argument);
void LEDToggleTask(void *argument);
void ImageProcessTask(void *argument);

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
  extern uint16_t* pCamera_Buffer;

  __attribute__((section(".ram_bss"))) __IO static uint16_t adcval[3] = {0};

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
    static TickType_t startTime = 0, endTime = 0;    
    endTime = xTaskGetTickCount();
    // sprintf(s, "ADC1: %d, ADC2: %d, ADC3: %d\n", adcval[0], adcval[1], adcval[2]);
    // CDC_Transmit_FS((uint8_t*)s, strlen(s));
    float Vref = vrefint_cal_vref / vrefint_cal_addr * adcval[2];
    float Vbat = adcval[0] * Vref / 65536.0f * 4.0f; // 电池电压，单位V
    float Temp = temperature_cal1_temp
                  + (adcval[1] * vrefint_cal_vref / Vref - tempsensor_cal1_addr)
                    * (temperature_cal2_temp - temperature_cal1_temp)
                    / (tempsensor_cal2_addr - tempsensor_cal1_addr);

    // LCD_Clear();

    LCD_ShowImage(0, 0, (uint16_t *)pCamera_Buffer);  // 将图像数据复制到屏幕
    LCD_printf(1, 1, "screen fps: %.1f, camera fps: %.1f", (1000.0f / (endTime - startTime)), OV5640_FPS);
    LCD_printf(2, 1, "%d", endTime);
    LCD_printf(3, 1, "VBat: %.1fV, Temp: %.1f^C, VREF: %.1fV", Vbat, Temp, Vref);
    LCD_SPI_Transmit();
    
    startTime = endTime;
    // osDelayUntil(endTime + 1000 / 30); // ~30fps
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
    osDelay(1000);
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
  extern uint16_t Camera_Buffer[2][Display_Height][Display_Width];
  extern uint16_t* pCamera_Buffer; // 摄像头数据缓冲区指针

  uint8_t i = 0;  // 缓冲区索引
  // extern uint16_t* pCamera_Buffer;

	DCMI_OV5640_Init();   			 	// DCMI以及OV5640初始化
	OV5640_AF_Download_Firmware();	// 写入自动对焦固件
	OV5640_AF_Trigger_Constant();		// 自动对焦 ，持续 触发，当OV5640检测到当前画面不在焦点时，会一直触发对焦
  OV5640_DMA_Transmit_Snapshot((uint32_t)Camera_Buffer[0], Display_BufferSize);  // 启动DMA快照传输

  /* Infinite loop */
  for(;;)
  {
    if(OV5640_FrameState == 1)  // 等待一帧数据传输完成
    {
      OV5640_FrameState = 0;  // 清除传输完成标志位
      pCamera_Buffer = (uint16_t*)Camera_Buffer[i]; // 切换到另一个缓冲区
      i = 1 - i;		 // 切换缓冲区索引
      OV5640_DCMI_Stop();
      OV5640_DMA_Transmit_Snapshot((uint32_t)Camera_Buffer[i], Display_BufferSize);  // 启动DMA快照传输      
      // 这里可以对图像数据进行处理，例如人脸识别、目标检测等
      // 目前只是简单地将摄像头数据复制到image缓冲区
      	
      	

    }
    else
    {
      osThreadYield();
    }
  }
  /* USER CODE END ImageProcessTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

