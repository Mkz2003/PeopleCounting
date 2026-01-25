#include "MyCode.h"
#include "main.h"
#include "usart.h"
#include <stdio.h>
#include "qspi_w25q64.h"




void Setup(void)
{
    QSPI_W25Qxx_Init();
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