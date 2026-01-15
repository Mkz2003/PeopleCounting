#include "MyCode.h"
#include "main.h"
#include "usart.h"
#include <stdio.h>




void Setup(void)
{

}


int fputc(int ch, FILE *f)
{
	HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 100);	// 发送单字节数据
	return (ch);
}



// int8_t CDC_Receive_Callback(uint8_t* Buf, uint32_t Len)
// {
    
// }