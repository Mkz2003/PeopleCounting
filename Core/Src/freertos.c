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
#include "stm32h7xx_hal.h"
#include "task.h"
#include "main.h"
#include "cmsis_os2.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "MyCode.h"
#include "usbd_def.h"

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

float Vref = 0.0f, Vbat = 0.0f, Temp = 0.0f; 
TickType_t DetectObject_Tick_Start = 0, DetectObject_Tick_End = 0;         	// 用于保存当前的时间计数值
float DetectObject_FPS = 0.0f, LCD_Tick_FPS = 0.0f;
array_t* faces = NULL,  * eyes = NULL, * faceIDs = NULL;
Polygon poly = {0};
struct
{
  int n;
  Point* vertices;
} polyMessage = {0};

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

  extern size_t umm_free_heap_size( void );
  extern size_t umm_max_free_block_size( void );
  extern int umm_usage_metric( void );
  extern int umm_fragmentation_metric( void );

  /* Infinite loop */
  for(;;)
  {
    TickType_t tick = xTaskGetTickCount();
    int8_t res;
    do
    {
      res = CDC_printf("[%d:%02d:%02d:%02d:%03d]\n"
                        "%d, %d, %d, %d\n"
                        "Vbat: %.3fV, Temp: %.3f^C, Vref: %.3fV\n"
                        "In: %d\n"
                        "Face: %d, OD: %.3f\n"
                        "\n",  
                        tick / 86400000, tick / 3600000 % 24, tick / 60000 % 60, tick / 1000 % 60, tick % 1000,
                        umm_free_heap_size(), umm_max_free_block_size(), umm_usage_metric(), umm_fragmentation_metric(),
                        Vbat, Temp, Vref,
                        poly.objectNum,
                        array_length(faceIDs), DetectObject_FPS);
      osDelay(20);
    }while(res != USBD_OK);

    osDelay(1000 / 2); // ~2fps
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
  extern uint16_t LCDBackImage[LCD_Height][LCD_Width];

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

    LCD_Tick_End = xTaskGetTickCount();
    LCD_Tick_FPS = 1000.0f / (LCD_Tick_End - LCD_Tick_Start);
    LCD_Tick_Start = LCD_Tick_End;

    // sprintf(s, "ADC1: %d, ADC2: %d, ADC3: %d\n", adcval[0], adcval[1], adcval[2]);
    // CDC_Transmit_FS((uint8_t*)s, strlen(s));
    Vref = vrefint_cal_vref / vrefint_cal_addr * adcval.Vref;
    Vbat = adcval.Vbat * Vref / (float)(1 << 16) * 4.0f;
    Temp = temperature_cal1_temp
                  + (adcval.Temp * vrefint_cal_vref / Vref - tempsensor_cal1_addr)
                    * (temperature_cal2_temp - temperature_cal1_temp)
                    / (tempsensor_cal2_addr - tempsensor_cal1_addr);

    image_t srcImg, midImg, dstImg;
    STM32Ipl_Init(&srcImg, Display_Width, Display_Height, IMAGE_BPP_RGB565, (uint8_t*)pCamera_Image);
    STM32Ipl_Init(&midImg, LCD_Height, LCD_Width, IMAGE_BPP_RGB565, (uint8_t*)LCDBackImage);
    STM32Ipl_Init(&dstImg, LCD_Height, LCD_Width, IMAGE_BPP_RGB565, (uint8_t*)LCD_Buff);
    // STM32Ipl_Resize(&srcImg, &dstImg, RESIZE_NEAREST);  // 将图像缩放到LCD分辨率，等价于LCD_ShowImage
    DMA2D_MemCopy_OS(srcImg.data, midImg.data, midImg.w, midImg.h, 0, 0, DMA2D_INPUT_RGB565);
    
    for (int i = 0; i < poly.n; ++i)
    {
      int n = poly.n;
      const Point* v = poly.vertices;
      const Point* a = &v[i];
      const Point* b = &v[(i + 1) % n];  // 边 a->b
      point_t _a = {.x = (int16_t)(a->x * LCD_Height), .y = (int16_t)(a->y * LCD_Width)};
      point_t _b = {.x = (int16_t)(b->x * LCD_Height), .y = (int16_t)(b->y * LCD_Width)};
      STM32Ipl_DrawLine(&midImg, &_a, &_b, STM32IPL_COLOR_GREEN, 1);
    }

    for (int i = 0; i < array_length(faceIDs); i++) // 一定要实时读faceIDs的状态，防止其他任务更改faceIDs的状态导致未定义行为
    {
      faceIDObject_t *f = array_at(faceIDs, i);
      // Draw the bounding box around each detected face.
      // uint16_t x = f->r.x * LCD_Height - (LCD_Height - LCD_Width) / 2;
      // uint16_t y = f->r.y * LCD_Height;
      // uint16_t w = f->r.width * LCD_Height;
      // uint16_t h = f->r.height * LCD_Height;
      uint16_t x = f->r.x * LCD_Height;
      uint16_t y = f->r.y * LCD_Width;
      uint16_t w = f->r.width * LCD_Height;
      uint16_t h = f->r.height * LCD_Width;
      stm32ipl_color_t color = f->color;
      STM32Ipl_DrawRectangle(&midImg, x, y, w, h, color, 2, false);
      STM32Ipl_DrawCircle(&midImg, (uint16_t)(f->point.x * LCD_Height), (uint16_t)(f->point.y * LCD_Width), 4, f->color, 1, 1);
    }

    STM32Ipl_Rotation270(&dstImg, &midImg);

    for (int i = 0; i < array_length(faceIDs); i++) // 一定要实时读faceIDs的状态，防止其他任务更改faceIDs的状态导致未定义行为
    {
      faceIDObject_t *f = array_at(faceIDs, i);
      uint16_t x = f->r.x * LCD_Height;
      uint16_t y = f->r.y * LCD_Width;
      uint16_t w = f->r.width * LCD_Height;
      uint16_t h = f->r.height * LCD_Width;
      LCD_printf(y / 12, x / 6, "%d", f->id);
      LCD_printf(y / 12, (x + w) / 6, "%.2f", f->score);
      LCD_printf((y + h) / 12, (x + w) / 6, "%d", f->zone);
    }

    // LCD_Clear();
    // LCD_ShowImage(0, 0, (uint16_t *)LCDBackImage);  // 将图像数据复制到屏幕
    // LCD_ShowImage(0, 0, (uint16_t *)pCamera_Image);  // 将图像数据复制到屏幕
    // LCD_printf(16, 1,"%d, %d, %d, %d", umm_free_heap_size(), umm_max_free_block_size(), umm_usage_metric(), umm_fragmentation_metric());
    // LCD_printf(7, 1, "Time: %d:%02d:%02d:%02d:%03d", LCD_Tick_End / 86400000, LCD_Tick_End / 3600000 % 24, LCD_Tick_End / 60000 % 60, LCD_Tick_End / 1000 % 60, LCD_Tick_End % 1000);
    // LCD_printf(18, 1, "Vbat: %.3fV, Temp: %.3f^C, Vref: %.3fV", Vbat, Temp, Vref);
    LCD_printf(18, 1, "In: %d", poly.objectNum);
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

  extern CDC_Receive_t CDC_Receive;
  extern uint16_t* pCamera_Image; // 摄像头数据缓冲区指针
  extern uint16_t LCD_Buff[LCD_Height][LCD_Width];
  extern uint8_t LCD_SPI_LOCKED;
  extern uint8_t UserTxBufferFS[APP_TX_DATA_SIZE];

  JPEGEncodeInit();

  /* Infinite loop */
  for(;;)
  {
    if(CDC_Receive.received)  // 等待一帧数据传输完成
    {
      CDC_Receive.received = 0;
      uint8_t* Buf = CDC_Receive.Buf;
      uint32_t Len = CDC_Receive.Len;
      char* s_jpeg = "jpeg";
      char* s_uid = "uid";
      char* s_area = "area";

      if(strncmp(Buf, s_jpeg, strlen(s_jpeg)) == 0)
      {
        while(LCD_SPI_LOCKED != 1) osThreadYield(); // 等待一帧数据传输完成

        uint32_t dstSize = JPEGEncode((uint8_t*)LCD_Buff, Display_Height * Display_Width * 2);
        while(CDC_Transmit_FS(jpeg_buffer, dstSize) != USBD_OK) osThreadYield();
      }
      else if(strncmp(Buf, s_uid, strlen(s_uid)) == 0)
      {
        while(CDC_printf("uid: %08X %08X %08X\n", HAL_GetUIDw0(), HAL_GetUIDw1(), HAL_GetUIDw2()) != USBD_OK) osThreadYield();
      }
      else if(strncmp(Buf, s_area, strlen(s_area)) == 0)
      {
        int n = *(int*)(Buf + 4);
        // float* pf = (float*)(Buf + 8);
        // float sum = 0.0f;
        // for(int i = 0; i < n * 2; i++)
        // {
        //   sum += *pf++;
        // }
        // if(fabsf(sum - *pf) <= 1e-6)
        if(Len == sizeof(Point) * n + 8)
        {
          // send to poly
          if(polyMessage.n == 0 && polyMessage.vertices == NULL) // 判断消息是否被其它任务接收
          {
            Point* vertices = (Point*)xalloc(sizeof(Point) * n);
            if(vertices != NULL)
            {
              memcpy(vertices, Buf + 8, sizeof(Point) * n);
              polyMessage.vertices = vertices;
              polyMessage.n = n;
            }
          }
        }
      }
      else
      {
        memcpy(UserTxBufferFS, Buf, Len);
        while(CDC_Transmit_FS(UserTxBufferFS, Len) != USBD_OK) osThreadYield();
      }
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

	DCMI_OV5640_Init();   			 	// DCMI以及OV5640初始化
	OV5640_AF_Download_Firmware();	// 写入自动对焦固件
	OV5640_AF_Trigger_Constant();		// 自动对焦 ，持续 触发，当OV5640检测到当前画面不在焦点时，会一直触发对焦
  OV5640_StartImageCapture();

  static Point test_vertices[] = {{0.5f, 0.0f},
                      {0.996875f, 0.f},
                      {0.996875f, 0.995833333f},
                      {0.5f, 0.995833333f},
  };
  
  PolygonInit(&poly, test_vertices, sizeof(test_vertices) / sizeof(Point), 1);
  Point* vertices = test_vertices; // 存取现在导入Poly的vertices

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

      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, poly.objectNum > 0);

      if(polyMessage.n > 0 && polyMessage.vertices != NULL)
      {
        PolygonDelete(&poly);
        if(vertices != test_vertices)
        {
          xfree(vertices);
          poly.vertices = NULL;
        }
        PolygonInit(&poly, polyMessage.vertices, polyMessage.n, 1);
        // 对polyMessage消息清零，通知消息已接收
        polyMessage.n = 0;
        polyMessage.vertices = NULL;

        vertices = polyMessage.vertices;
      }
      
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

