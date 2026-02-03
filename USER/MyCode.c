#include "MyCode.h"
#include "main.h"
#include "usart.h"
#include <stdio.h>
#include "qspi_w25q64.h"




void Setup(void)
{
    QSPI_W25Qxx_Init();
    // QSPI_W25Qxx_ChipErase();
    int QSPI_Status = QSPI_W25Qxx_MemoryMappedMode(); // 配置QSPI为内存映射模式
	if( QSPI_Status == QSPI_W25Qxx_OK )
	{
		printf ("\r\n进入内存映射模式成功，开始读取>>>>\r\n");		
	}
	else
	{
		printf ("\r\n内存映射错误！！  错误代码:%d\r\n",QSPI_Status);
		while (1);
	}
}

#ifdef __GNUC__
int __io_putchar(int ch)
{
#else
int fputc(int ch, FILE *f)
{
#endif
    // 发送字符到串口
    HAL_UART_Transmit(&huart1, (uint8_t*)&ch, 1, HAL_MAX_DELAY);
    
    // 如果需要回显，也可以处理回车换行
    if (ch == '\n')
    {
        uint8_t ret = '\r';
        HAL_UART_Transmit(&huart1, &ret, 1, HAL_MAX_DELAY);
    }
    
    return ch;
}


// int8_t CDC_Receive_Callback(uint8_t* Buf, uint32_t Len)
// {
    
// }