 /***
	************************************************************************************************************************************************************************************************
	*	@version V2.0
	*  @date    2026-1-2
	*	@author  反客科技（mkz modified）
   *************************************************************************************************
   *  @description
	*
	*	实验平台：反客STM32H743XIH6核心板 （型号：FK743M5-XIH6）+ 2.00寸液晶模块（型号：SPI200M1-240*320）
	*	淘宝地址：https://shop212360197.taobao.com
	*	QQ交流群：536665479
	*
>>>>> 重要说明：
	*
	*  1.屏幕配置为16位RGB565格式
	*  2.SPI通信速度为60M
	*********************************************************************************************************************************************************************************************FANKE*****
***/
#include "cmsis_os2.h"
#include "main.h"
#include "lcd_spi_200.h"
#include "lcd_font.h"
#include "DMA2DProcess.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

extern SPI_HandleTypeDef hspi5;	      // SPI_HandleTypeDef 结构体变量

#define  LCD_SPI hspi5           // SPI局部宏，方便修改和移植

//SPI传输相关
static void LCD_SetAddress(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2);
volatile uint8_t LCD_SPI_LOCKED = 0;   // SPI总线锁定标志，0表示空闲，1表示锁定中

__attribute__((section(".ram_d2_bss"), aligned(32))) uint16_t LCD_Buff[LCD_Height][LCD_Width] = {0};        // LCD缓冲区，16位宽（每个像素点占2字节）
__attribute__((section(".sdram_bss"), aligned(32))) uint16_t LCDBackImage[LCD_Height][LCD_Width] = {0};        // 图像处理缓冲区，16位宽（每个像素点占2字节）

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if(hspi->Instance == LCD_SPI.Instance) // 判断是否是LCD使用的SPI
	{
        LCD_SPI_LOCKED = 0;
	}
}

//SPI传输相关

/****************************************************************************************************************************************
*	函 数 名: LCD_SPI_Transmit
*
*	入口参数: None
*
*	函数功能: 批量写入数据到屏幕
*	
****************************************************************************************************************************************/
void LCD_SPI_Transmit()
{
    //This function transmits the data in the LCD_Buff buffer to the LCD screen using DMA in 16-bit data width, big-endian format.
    //此函数以16bit数据宽度通过DMA方式批量传输LCD_Buff缓冲区的数据到屏幕，大端优先。
    uint32_t Size = 0;
    uint32_t chunkSize = sizeof(LCD_Buff) / 2;                   // 每次传输的数据块大小
    uint8_t *pData = (uint8_t*)LCD_Buff; // 数据指针，指向LCD缓冲区

    while(1)
    {
        if(LCD_SPI_LOCKED == 0)
        {
            if(Size < sizeof(LCD_Buff) - chunkSize) //剩余传输数据大于chunkSize
            {
                LCD_SPI_LOCKED = 1;  // 锁定SPI总线
                HAL_SPI_Transmit_DMA(&LCD_SPI, pData + Size, chunkSize / 2);
                Size += chunkSize;
            }
            else if(Size < sizeof(LCD_Buff))    //剩余传输数据小于chunkSize
            {
                LCD_SPI_LOCKED = 1;  // 锁定SPI总线
                HAL_SPI_Transmit_DMA(&LCD_SPI, pData + Size, (sizeof(LCD_Buff) - Size) / 2);
                Size = sizeof(LCD_Buff);
            }
            else    // 全部数据传输完毕
            {
                break;  // 退出传输循环
            }
        }
        else
        {
            #ifdef CMSIS_OS2_H_
            osThreadYield();  // 让出CPU时间片
            #endif
        }
    }
}


struct	//LCD相关参数结构体
{
    uint32_t Color;  				//	LCD当前画笔颜色
    uint32_t BackColor;			//	背景色
    uint8_t  Direction;			//	显示方向
    uint16_t Width;            // 屏幕像素长度
    uint16_t Height;           // 屏幕像素宽度	
    uint8_t  X_Offset;         // X坐标偏移，用于设置屏幕控制器的显存写入方式
    uint8_t  Y_Offset;         // Y坐标偏移，用于设置屏幕控制器的显存写入方式
}LCD;

/******************************************************  

		PK0     ------> SPI5_SCK
		PJ10    ------> SPI5_MOSI
		PH5     ------> SPI5_NSS

		PH6     ------> 背光  引脚
		PJ11    ------> 数据指令选择 引脚
		
*******************************************************/

/****************************************************************************************************************************************
*	函 数 名: LCD_WriteCommand
*
*	入口参数: lcd_command - 需要写入的控制指令
*
*	函数功能: 用于向屏幕控制器写入指令
*
****************************************************************************************************************************************/

void  LCD_WriteCommand(uint8_t lcd_command)
{
	// while(LCD_SPI_LOCKED);  // 等待SPI总线空闲
   LCD_DC_Command;     // 数据指令选择 引脚输出低电平，代表本次传输 指令
	
	// LCD_SPI_LOCKED = 1;  // 锁定SPI总线
   HAL_SPI_Transmit(&LCD_SPI, &lcd_command, 1, 1000); // 启动SPI传输
   

}

/****************************************************************************************************************************************
*	函 数 名: LCD_WriteData_8bit
*
*	入口参数: lcd_data - 需要写入的数据，8位
*
*	函数功能: 写入8位数据
*	
****************************************************************************************************************************************/

void  LCD_WriteData_8bit(uint8_t lcd_data)
{
	// while(LCD_SPI_LOCKED);  // 等待SPI总线空闲
   LCD_DC_Data;     // 数据指令选择 引脚输出高电平，代表本次传输 数据

	// LCD_SPI_LOCKED = 1;  // 锁定SPI总线
   HAL_SPI_Transmit(&LCD_SPI, &lcd_data, 1, 1000); // 启动SPI传输
   
}

/****************************************************************************************************************************************
*	函 数 名: LCD_WriteData_16bit
*
*	入口参数: lcd_data - 需要写入的数据，16位
*
*	函数功能: 写入16位数据
*	
****************************************************************************************************************************************/

void  LCD_WriteData_16bit(uint16_t lcd_data)
{
   	static uint8_t lcd_data_buff[2];    // 数据发送区

	// while(LCD_SPI_LOCKED);  // 等待SPI总线空闲
   	LCD_DC_Data;      // 数据指令选择 引脚输出高电平，代表本次传输 数据
   	lcd_data_buff[0] = lcd_data>>8;  // 将数据拆分
   	lcd_data_buff[1] = lcd_data;

	// LCD_SPI_LOCKED = 1;  // 锁定SPI总线		
	HAL_SPI_Transmit(&LCD_SPI, lcd_data_buff, 2, 1000) ;   // 启动SPI传输
	
}

/****************************************************************************************************************************************
*	函 数 名: SPI_LCD_Init
*
*	函数功能: 初始化SPI以及屏幕控制器的各种参数
*	
****************************************************************************************************************************************/

void SPI_LCD_Init(void)
{
    LCD_Backlight_OFF; // 关闭背光
   	HAL_Delay(10);               	// 屏幕刚完成复位时（包括上电复位），需要等待至少5ms才能发送指令
 	LCD_WriteCommand(0x36);       // 显存访问控制 指令，用于设置访问显存的方式
	LCD_WriteData_8bit(0x00);     // 配置成 从上到下、从左到右，RGB像素格式

	LCD_WriteCommand(0x3A);			// 接口像素格式 指令，用于设置使用 12位、16位还是18位色
	LCD_WriteData_8bit(0x05);     // 此处配置成 16位 像素格式

// 接下来很多都是电压设置指令，直接使用厂家给设定值
 	LCD_WriteCommand(0xB2);			
	LCD_WriteData_8bit(0x0C);
	LCD_WriteData_8bit(0x0C); 
	LCD_WriteData_8bit(0x00); 
	LCD_WriteData_8bit(0x33); 
	LCD_WriteData_8bit(0x33); 			

	LCD_WriteCommand(0xB7);		   // 栅极电压设置指令	
	LCD_WriteData_8bit(0x35);     // VGH = 13.26V，VGL = -10.43V

	LCD_WriteCommand(0xBB);			// 公共电压设置指令
	LCD_WriteData_8bit(0x19);     // VCOM = 1.35V

	LCD_WriteCommand(0xC0);
	LCD_WriteData_8bit(0x2C);

	LCD_WriteCommand(0xC2);       // VDV 和 VRH 来源设置
	LCD_WriteData_8bit(0x01);     // VDV 和 VRH 由用户自由配置

	LCD_WriteCommand(0xC3);			// VRH电压 设置指令  
	LCD_WriteData_8bit(0x12);     // VRH电压 = 4.6+( vcom+vcom offset+vdv)
				
	LCD_WriteCommand(0xC4);		   // VDV电压 设置指令	
	LCD_WriteData_8bit(0x20);     // VDV电压 = 0v

	LCD_WriteCommand(0xC6); 		// 正常模式的帧率控制指令
	LCD_WriteData_8bit(0x00);   	// 设置屏幕控制器的刷新帧率为119帧    

	LCD_WriteCommand(0xD0);			// 电源控制指令
	LCD_WriteData_8bit(0xA4);     // 无效数据，固定写入0xA4
	LCD_WriteData_8bit(0xA1);     // AVDD = 6.8V ，AVDD = -4.8V ，VDS = 2.3V

	LCD_WriteCommand(0xE0);       // 正极电压伽马值设定
	LCD_WriteData_8bit(0xD0);
	LCD_WriteData_8bit(0x04);
	LCD_WriteData_8bit(0x0D);
	LCD_WriteData_8bit(0x11);
	LCD_WriteData_8bit(0x13);
	LCD_WriteData_8bit(0x2B);
	LCD_WriteData_8bit(0x3F);
	LCD_WriteData_8bit(0x54);
	LCD_WriteData_8bit(0x4C);
	LCD_WriteData_8bit(0x18);
	LCD_WriteData_8bit(0x0D);
	LCD_WriteData_8bit(0x0B);
	LCD_WriteData_8bit(0x1F);
	LCD_WriteData_8bit(0x23);

	LCD_WriteCommand(0xE1);      // 负极电压伽马值设定
	LCD_WriteData_8bit(0xD0);
	LCD_WriteData_8bit(0x04);
	LCD_WriteData_8bit(0x0C);
	LCD_WriteData_8bit(0x11);
	LCD_WriteData_8bit(0x13);
	LCD_WriteData_8bit(0x2C);
	LCD_WriteData_8bit(0x3F);
	LCD_WriteData_8bit(0x44);
	LCD_WriteData_8bit(0x51);
	LCD_WriteData_8bit(0x2F);
	LCD_WriteData_8bit(0x1F);
	LCD_WriteData_8bit(0x1F);
	LCD_WriteData_8bit(0x20);
	LCD_WriteData_8bit(0x23);

	LCD_WriteCommand(0x21);       // 打开反显，因为面板是常黑型，操作需要反过来

 // 退出休眠指令，LCD控制器在刚上电、复位时，会自动进入休眠模式 ，因此操作屏幕之前，需要退出休眠  
	LCD_WriteCommand(0x11);       // 退出休眠 指令
   	HAL_Delay(120);               // 需要等待120ms，让电源电压和时钟电路稳定下来

 // 打开显示指令，LCD控制器在刚上电、复位时，会自动关闭显示 
	LCD_WriteCommand(0x29);       // 打开显示
    
    LCD_SetDirection(Direction_V);  	      //	设置显示方向

// 以下进行一些驱动的默认设置
    LCD_SetAddress(0, 0, LCD_Width - 1, LCD_Height - 1); // 设置显示区域为全屏

    // 修改为16位数据宽度，大端优先，与LCD屏所需格式相符
    LCD_SPI.Init.DataSize 	= SPI_DATASIZE_16BIT;   //	16位数据宽度
    HAL_SPI_Init(&LCD_SPI);	
    
	LCD_SetBackColor(LCD_BLACK);           // 设置背景色
 	LCD_SetColor(LCD_WHITE);               // 设置画笔色  
	LCD_Clear();                           // 清屏

    LCD_DC_Data;     // 数据指令选择 引脚输出高电平，代表本次传输 数据
    LCD_SPI_Transmit();
    HAL_Delay(500);

// 全部设置完毕之后，打开背光	
   LCD_Backlight_ON;  // 引脚输出高电平点亮背光
}

/****************************************************************************************************************************************
*	函 数 名:	 LCD_SetAddress
*
*	入口参数:	 x1 - 起始水平坐标   y1 - 起始垂直坐标  
*              x2 - 终点水平坐标   y2 - 终点垂直坐标	   
*	
*	函数功能:   设置需要显示的坐标区域		 			 
*****************************************************************************************************************************************/

static void LCD_SetAddress(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2)		
{
	LCD_WriteCommand(0x2a);			//	列地址设置，即X坐标
	LCD_WriteData_16bit(x1+LCD.X_Offset);
	LCD_WriteData_16bit(x2+LCD.X_Offset);

	LCD_WriteCommand(0x2b);			//	行地址设置，即Y坐标
	LCD_WriteData_16bit(y1+LCD.Y_Offset);
	LCD_WriteData_16bit(y2+LCD.Y_Offset);

	LCD_WriteCommand(0x2c);			//	开始写入显存，即要显示的颜色数据
}

/****************************************************************************************************************************************
*	函 数 名:	LCD_SetColor
*
*	入口参数:	Color - 要显示的颜色，示例：0x0000FF 表示蓝色
*
*	函数功能:	此函数用于设置画笔的颜色，例如显示字符、画点画线、绘图的颜色
*
*	说    明:	1. 为了方便用户使用自定义颜色，入口参数 Color 使用24位 RGB888的颜色格式，用户无需关心颜色格式的转换
*					2. 24位的颜色中，从高位到低位分别对应 R、G、B  3个颜色通道
*
*****************************************************************************************************************************************/

void LCD_SetColor(uint32_t Color)
{
	uint16_t Red_Value = 0, Green_Value = 0, Blue_Value = 0; //各个颜色通道的值

	Red_Value   = (uint16_t)((Color&0x00F80000)>>8);   // 转换成 16位 的RGB565颜色
	Green_Value = (uint16_t)((Color&0x0000FC00)>>5);
	Blue_Value  = (uint16_t)((Color&0x000000F8)>>3);

	LCD.Color = (uint16_t)(Red_Value | Green_Value | Blue_Value);  // 将颜色写入全局LCD参数		
}

/****************************************************************************************************************************************
*	函 数 名:	LCD_SetBackColor
*
*	入口参数:	Color - 要显示的颜色，示例：0x0000FF 表示蓝色
*
*	函数功能:	设置背景色,此函数用于清屏以及显示字符的背景色
*
*	说    明:	1. 为了方便用户使用自定义颜色，入口参数 Color 使用24位 RGB888的颜色格式，用户无需关心颜色格式的转换
*					2. 24位的颜色中，从高位到低位分别对应 R、G、B  3个颜色通道
*
*****************************************************************************************************************************************/

void LCD_SetBackColor(uint32_t Color)
{
	uint16_t Red_Value = 0, Green_Value = 0, Blue_Value = 0; //各个颜色通道的值

	Red_Value   = (uint16_t)((Color&0x00F80000)>>8);   // 转换成 16位 的RGB565颜色
	Green_Value = (uint16_t)((Color&0x0000FC00)>>5);
	Blue_Value  = (uint16_t)((Color&0x000000F8)>>3);

	LCD.BackColor = (uint16_t)(Red_Value | Green_Value | Blue_Value);	// 将颜色写入全局LCD参数			   	
}

/****************************************************************************************************************************************
*	函 数 名:	LCD_SetDirection
*
*	入口参数:	direction - 要显示的方向
*
*	函数功能:	设置要显示的方向
*
*	说    明:   1. 可输入参数 Direction_H 、Direction_V 、Direction_H_Flip 、Direction_V_Flip        
*              2. 使用示例 LCD_DisplayDirection(Direction_H) ，即设置屏幕横屏显示
*
*****************************************************************************************************************************************/

void LCD_SetDirection(uint8_t direction)
{
	LCD.Direction = direction;    // 写入全局LCD参数

   if( direction == Direction_H )   // 横屏显示
   {
      LCD_WriteCommand(0x36);    		// 显存访问控制 指令，用于设置访问显存的方式
      LCD_WriteData_8bit(0x70);        // 横屏显示
      LCD.X_Offset   = 0;             // 设置控制器坐标偏移量
      LCD.Y_Offset   = 0;   
      LCD.Width      = LCD_Height;		// 重新赋值长、宽
      LCD.Height     = LCD_Width;		
   }
   else if( direction == Direction_V )
   {
      LCD_WriteCommand(0x36);    		// 显存访问控制 指令，用于设置访问显存的方式
      LCD_WriteData_8bit(0x00);        // 垂直显示
      LCD.X_Offset   = 0;              // 设置控制器坐标偏移量
      LCD.Y_Offset   = 0;     
      LCD.Width      = LCD_Width;		// 重新赋值长、宽
      LCD.Height     = LCD_Height;						
   }
   else if( direction == Direction_H_Flip )
   {
      LCD_WriteCommand(0x36);   			 // 显存访问控制 指令，用于设置访问显存的方式
      LCD_WriteData_8bit(0xA0);         // 横屏显示，并上下翻转，RGB像素格式
      LCD.X_Offset   = 0;              // 设置控制器坐标偏移量
      LCD.Y_Offset   = 0;      
      LCD.Width      = LCD_Height;		 // 重新赋值长、宽
      LCD.Height     = LCD_Width;				
   }
   else if( direction == Direction_V_Flip )
   {
      LCD_WriteCommand(0x36);    		// 显存访问控制 指令，用于设置访问显存的方式
      LCD_WriteData_8bit(0xC0);        // 垂直显示 ，并上下翻转，RGB像素格式
      LCD.X_Offset   = 0;              // 设置控制器坐标偏移量
      LCD.Y_Offset   = 0;     
      LCD.Width      = LCD_Width;		// 重新赋值长、宽
      LCD.Height     = LCD_Height;				
   }   
}

/**
  * @brief          operate the graphic ram(size: 128*8 char)
  * @param[in]      pen: the type of operate.
                    PEN_CLEAR: set ram to 0x00
                    PEN_WRITE: set ram to 0xff
                    PEN_INVERSION: bit inversion 
  * @retval         none
  */
/**
  * @brief          操作GRAM内存(128*8char数组)
  * @param[in]      pen: 操作类型.
                    PEN_CLEAR: 设置为0x00
                    PEN_WRITE: 设置为0xff
                    PEN_INVERSION: 按位取反
  * @retval         none
  */
void LCD_Clear(void)
{
	uint16_t i, j;

	for (i = 0; i < LCD_Height; i++)
	{
		for (j = 0; j < LCD_Width; j++)
		{
			LCD_Buff[i][j] = LCD.BackColor;
		}
	}
}


/**
  * @brief          操作GRAM中的一个位，相当于操作屏幕的一个点
  * @param[in]      x:x轴,  [0,X_WIDTH-1]
  * @param[in]      y:y轴,  [0,Y_WIDTH-1]
  * @param[in]      pen: 操作类型,
                        PEN_CLEAR: 设置 (x,y) 点为 0
                        PEN_WRITE: 设置 (x,y) 点为 1
                        PEN_INVERSION: (x,y) 值反转
  * @retval         none
  */
void LCD_draw_point(uint16_t x, uint16_t y)
{

    /* check the corrdinate */
    if ((x < 0) || (x > (LCD_Height - 1)) || (y < 0) || (y > (LCD_Width - 1)))
    {
        return;
    }
	LCD_Buff[LCD_Height - 1 - x][y] = LCD.Color;
}




/**
  * @brief          draw a line from (x1, y1) to (x2, y2)
  * @param[in]      x1: the start point of line
  * @param[in]      y1: the start point of line
  * @param[in]      x2: the end point of line
  * @param[in]      y2: the end point of line
  * @param[in]      pen: type of operation,PEN_CLEAR,PEN_WRITE,PEN_INVERSION.
  * @retval         none
  */
/**
  * @brief          画一条直线，从(x1,y1)到(x2,y2)
  * @param[in]      x1: 起点
  * @param[in]      y1: 起点
  * @param[in]      x2: 终点
  * @param[in]      y2: 终点
  * @param[in]      pen: 操作类型,PEN_CLEAR,PEN_WRITE,PEN_INVERSION.
  * @retval         none
  */

void LCD_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    uint16_t col = 0, row = 0;
    uint16_t x_st = 0, x_ed = 0, y_st = 0, y_ed = 0;
    float k = 0.0f, b = 0.0f;

    if (y1 == y2)
    {
        (x1 <= x2) ? (x_st = x1):(x_st = x2);
        (x1 <= x2) ? (x_ed = x2):(x_ed = x1);

        for (col = x_st; col <= x_ed; col++)
        {
            LCD_draw_point(col, y1);
        }
    }
    else if (x1 == x2)
    {
        (y1 <= y2) ? (y_st = y1):(y_st = y2);
        (y1 <= y2) ? (y_ed = y2):(y_ed = y1);

        for (row = y_st; row <= y_ed; row++)
        {
            LCD_draw_point(x1, row);
        }
    }
    else
    {
        k = ((float)(y2 - y1)) / (x2 - x1);
        b = (float)y1 - k * x1;

        (x1 <= x2) ? (x_st = x1):(x_st = x2);
        (x1 <= x2) ? (x_ed = x2):(x_ed = x2);

        for (col = x_st; col <= x_ed; col++)
        {
            LCD_draw_point(col, (uint8_t)(col * k + b));
        }
    }
}


/**
  * @brief          show a character
  * @param[in]      row: start row of character
  * @param[in]      col: start column of character
  * @param[in]      chr: the character ready to show
  * @retval         none
  */
/**
  * @brief          显示一个字符
  * @param[in]      row: 字符的开始行
  * @param[in]      col: 字符的开始列
  * @param[in]      chr: 字符
  * @retval         none
  */
void LCD_show_char(uint16_t row, uint16_t col, uint16_t chr)
{
    uint16_t x = col * 6;
    uint16_t y = row * 12;
    uint16_t temp, t, t1;
    uint16_t y0 = y;
    chr = chr - ' ';

    if(row >= (LCD_Width / 12) || col >= (LCD_Height / 6))
    {
        return;
    }
    for (t = 0; t < 12; t++)
    {
        temp = asc2_1206[chr][t];

        for (t1 = 0; t1 < 8; t1++)
        {
            if (temp&0x80)
                LCD_draw_point(x, y);

            temp <<= 1;
            y++;
            if ((y - y0) == 12)
            {
                y = y0;
                x++;
                break;
            }
        }
    }
}


/**
  * @brief          show a character string
  * @param[in]      row: row of character string begin
  * @param[in]      col: column of character string begin
  * @param[in]      chr: the pointer to character string
  * @retval         none
  */
/**
  * @brief          显示一个字符串
  * @param[in]      row: 字符串的开始行
  * @param[in]      col: 字符串的开始列
  * @param[in]      chr: 字符串
  * @retval         none
  */
void LCD_show_string(uint16_t row, uint16_t col, uint8_t *chr)
{
    uint16_t n = 0;

    while (chr[n] != '\0')
    {
        LCD_show_char(row, col, chr[n]);
        col++;

        if (col >= LCD_Height / 6)
        {
            col = 0;
            row += 1;
        }
        n++;
    }
}


/**
  * @brief          formatted output in LCD 128*64
  * @param[in]      row: row of character string begin, 0 <= row <= 4;
  * @param[in]      col: column of character string begin, 0 <= col <= 20;
  * @param          *fmt: the pointer to format character string
  * @note           if the character length is more than one row at a time, the extra characters will be truncated
  * @retval         none
  */
/**
  * @brief          格式输出
  * @param[in]      row: 开始列，0 <= row <= 4;
  * @param[in]      col: 开始行， 0 <= col <= 20;
  * @param[in]      *fmt:格式化输出字符串
  * @note           如果字符串长度大于一行，额外的字符会换行
  * @retval         none
  */
void LCD_printf(uint16_t row, uint16_t col, const char *fmt,...)
{
    static uint8_t LCD_BUF[128] = {0};
    static va_list ap;
    va_start(ap, fmt);

    vsprintf((char *)LCD_BUF, fmt, ap);

    va_end(ap);

    LCD_BUF[127] = '\0';

    LCD_show_string(row, col, LCD_BUF);
}

/**
  * @brief          show the image
  * @param[in]      none
  * @retval         none
  */
/**
  * @brief          显示图像RGB565格式
  * @param[in]      none
  * @retval         none
  */
void LCD_ShowImage(uint16_t x, uint16_t y, uint16_t* image)
{
    DMA2D_MemCopy(image, LCD_Buff, LCD_Width, LCD_Height, 0, 0, DMA2D_OUTPUT_RGB565);
    // memcpy(LCD_Buff, image, sizeof(LCD_Buff));

}
