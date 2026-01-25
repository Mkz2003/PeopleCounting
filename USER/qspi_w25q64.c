/****
	***********************************************************************************************************************************************************************************
	*	@version V1.0
	*  @date    2023-12-13
	*	@author  反客科技	
   *************************************************************************************************
   *  @description
	*
	*	实验平台：反客STM32H743XIH6核心板 （型号：FK743M5-XIH6）
	*	淘宝地址：https://shop212360197.taobao.com
	*	QQ交流群：536665479
	*
>>>>> 文件说明：
	*
	*  1.例程参考于官方驱动文件 stm32h743i_eval_qspi.c
	*	2.例程使用的是 QUADSPI_BK1
	*	3.提供的读写函数均使用HAL库函数直接操作，没有用到DMA和中断
	*	4.默认配置QSPI驱动时钟为120M
	*
>>>>> 重要说明：
	*
	*	1.W25QXX的擦除时间是限定的!!! 手册给出的典型参考值为: 4K-45ms, 32K-120ms ,64K-150ms,整片擦除20S
	*
	*	2.W25QXX的写入时间是限定的!!! 手册给出的典型参考值为: 256字节-0.4ms，也就是 1M字节/s （实测大概在600K字节/s左右）
	*
	*	3.如果使用库函数直接读取，那么是否使用DMA、是否开启Cache、编译器的优化等级以及数据存储区的位置(内部 TCM SRAM 或者 AXI SRAM)都会影响读取的速度
	*
	*	4.如果使用内存映射模式，则读取性能只与QSPI的驱动时钟以及是否开启Cache有关
	*
	*	5.使用库函数进行直接读取，keil版本5.30，编译器AC6.14，编译等级Oz image size，读取速度为 7M字节/S ，数据放在TCM SRAM 或者 AXI SRAM  
	*    都是差不多的结果，因为CPU直接访问外设寄存器的效率很低，直接使用HAL库进行读取的话，速度很慢
	*
	*	6.如果使用MDMA进行读取，可以达到 58M字节/S，使用内存映射模式的话，几乎可以达到驱动时钟的全速，62.14M/s (133MHz时钟下)
	*
	*  7.W25Q64JV 所允许的最高驱动频率为133MHz，750的QSPI最高驱动频率也是133MHz ，但是对于HAL库函数直接读取而言，驱动时钟超过15M已经不会有性能提升
	*
	*	8.对于内存映射模式直接读取而言，驱动时钟超过127.5M已经不会有性能提升，因为QSPI内核时钟最高限定为250M，所以建议实际QSPI驱动时钟不要超过125M，
	*	  具体的时钟配置请参考 SystemClock_Config 函数
	*
	*	9.实际使用中，当数据比较大时，建议使用64K或者32K擦除，擦除时间比4K擦除块	
	*
	**************************************************************************************************************************************************************************************FANKE*****
***/

#include "qspi_w25q64.h"
#include <stdio.h>

extern QSPI_HandleTypeDef hqspi;	// 定义QSPI句柄，这里保留使用cubeMX生成的变量命名，方便用户参考和移植

/*************************************************************************************************
*	函 数 名: QSPI_W25Qxx_Init
*	入口参数: 无
*	返 回 值: QSPI_W25Qxx_OK - 初始化成功，W25Qxx_ERROR_INIT - 初始化错误
*	函数功能: 初始化 QSPI 配置，读取W25Q64ID
*	说    明: 无	
*************************************************************************************************/

int8_t QSPI_W25Qxx_Init(void)
{
	uint32_t	Device_ID;	// 器件ID
	
	QSPI_W25Qxx_Reset();							// 复位器件
	Device_ID = QSPI_W25Qxx_ReadID(); 		// 读取器件ID
	
	if( Device_ID == W25Qxx_FLASH_ID )		// 进行匹配
	{
		printf("W25Q64 OK,flash ID:%X\r\n",Device_ID);		// 初始化成功	
	}
	else
	{
		printf("W25Q64 ERROR!!!!!  ID:%X\r\n",Device_ID);	// 初始化失败	
		return W25Qxx_ERROR_INIT;		// 返回错误标志
	}

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
	return QSPI_W25Qxx_OK;			// 返回成功标志	
}

/*************************************************************************************************
*	函 数 名: QSPI_W25Qxx_AutoPollingMemReady
*	入口参数: 无
*	返 回 值: QSPI_W25Qxx_OK - 通信正常结束，W25Qxx_ERROR_AUTOPOLLING - 轮询等待无响应
*	函数功能: 使用自动轮询标志查询，等待通信结束
*	说    明: 每一次通信都应该调用次函数，等待通信结束，避免错误的操作	
**************************************************************************************************/

int8_t QSPI_W25Qxx_AutoPollingMemReady(void)
{
	QSPI_CommandTypeDef     s_command;	   // QSPI传输配置
	QSPI_AutoPollingTypeDef s_config;		// 轮询比较相关配置参数

	s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;			// 1线指令模式
	s_command.AddressMode       = QSPI_ADDRESS_NONE;					// 无地址模式
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;			//	无交替字节 
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;	     	 	// 禁止DDR模式
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;	   	// DDR模式中数据延迟，这里用不到
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;	   	//	每次传输数据都发送指令	
	s_command.DataMode          = QSPI_DATA_1_LINE;						// 1线数据模式
	s_command.DummyCycles       = 0;											//	空周期个数
	s_command.Instruction       = W25Qxx_CMD_ReadStatus_REG1;	   // 读状态信息寄存器
																					
// 不停的查询 W25Qxx_CMD_ReadStatus_REG1 寄存器，将读取到的状态字节中的 W25Qxx_Status_REG1_BUSY 不停的与0作比较
// 读状态寄存器1的第0位（只读），Busy标志位，当正在擦除/写入数据/写命令时会被置1，空闲或通信结束为0
	
	s_config.Match           = 0;   									//	匹配值
	s_config.MatchMode       = QSPI_MATCH_MODE_AND;	      	//	与运算
	s_config.Interval        = 0x10;	                     	//	轮询间隔
	s_config.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;	// 自动停止模式
	s_config.StatusBytesSize = 1;	                        	//	状态字节数
	s_config.Mask            = W25Qxx_Status_REG1_BUSY;	   // 对在轮询模式下接收的状态字节进行屏蔽，只比较需要用到的位
		
	// 发送轮询等待命令
	if (HAL_QSPI_AutoPolling(&hqspi, &s_command, &s_config, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return W25Qxx_ERROR_AUTOPOLLING; // 轮询等待无响应
	}
	return QSPI_W25Qxx_OK; // 通信正常结束

}

/*************************************************************************************************
*	函 数 名: QSPI_W25Qxx_Reset
*	入口参数: 无
*	返 回 值: QSPI_W25Qxx_OK - 复位成功，W25Qxx_ERROR_INIT - 初始化错误
*	函数功能: 复位器件
*	说    明: 无	
*************************************************************************************************/

int8_t QSPI_W25Qxx_Reset(void)	
{
	QSPI_CommandTypeDef s_command;	// QSPI传输配置

	s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;   	// 1线指令模式
	s_command.AddressMode 		 = QSPI_ADDRESS_NONE;   			// 无地址模式
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE; 	// 无交替字节 
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;     	// 禁止DDR模式
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY; 	// DDR模式中数据延迟，这里用不到
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;	 	// 每次传输数据都发送指令
	s_command.DataMode 			 = QSPI_DATA_NONE;       			// 无数据模式	
	s_command.DummyCycles 		 = 0;                     			// 空周期个数
	s_command.Instruction 		 = W25Qxx_CMD_EnableReset;       // 执行复位使能命令

	// 发送复位使能命令
	if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) 
	{
		return W25Qxx_ERROR_INIT;			// 如果发送失败，返回错误信息
	}
	// 使用自动轮询标志位，等待通信结束
	if (QSPI_W25Qxx_AutoPollingMemReady() != QSPI_W25Qxx_OK)
	{
		return W25Qxx_ERROR_AUTOPOLLING;	// 轮询等待无响应
	}

	s_command.Instruction  = W25Qxx_CMD_ResetDevice;     // 复位器件命令    

	//发送复位器件命令
	if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) 
	{
		return W25Qxx_ERROR_INIT;		  // 如果发送失败，返回错误信息
	}
	// 使用自动轮询标志位，等待通信结束
	if (QSPI_W25Qxx_AutoPollingMemReady() != QSPI_W25Qxx_OK)
	{
		return W25Qxx_ERROR_AUTOPOLLING;	// 轮询等待无响应
	}	
	return QSPI_W25Qxx_OK;	// 复位成功
}

/*************************************************************************************************
*	函 数 名: QSPI_W25Qxx_ReadID
*	入口参数: 无
*	返 回 值: W25Qxx_ID - 读取到的器件ID，W25Qxx_ERROR_INIT - 通信、初始化错误
*	函数功能: 初始化 QSPI 配置，读取器件ID
*	说    明: 无	
**************************************************************************************************/

uint32_t QSPI_W25Qxx_ReadID(void)	
{
	QSPI_CommandTypeDef s_command;	// QSPI传输配置
	uint8_t	QSPI_ReceiveBuff[3];		// 存储QSPI读到的数据
	uint32_t	W25Qxx_ID;					// 器件的ID

	s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    // 1线指令模式
	s_command.AddressSize       = QSPI_ADDRESS_24_BITS;     	 // 24位地址
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  // 无交替字节 
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;      // 禁止DDR模式
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  // DDR模式中数据延迟，这里用不到
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;	 // 每次传输数据都发送指令
	s_command.AddressMode		 = QSPI_ADDRESS_NONE;   		 // 无地址模式
	s_command.DataMode			 = QSPI_DATA_1_LINE;       	 // 1线数据模式
	s_command.DummyCycles 		 = 0;                   		 // 空周期个数
	s_command.NbData 				 = 3;                          // 传输数据的长度
	s_command.Instruction 		 = W25Qxx_CMD_JedecID;         // 执行读器件ID命令

	// 发送指令
	if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) 
	{
//		return W25Qxx_ERROR_INIT;		// 如果发送失败，返回错误信息
	}
	// 接收数据
	if (HAL_QSPI_Receive(&hqspi, QSPI_ReceiveBuff, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) 
	{
//		return W25Qxx_ERROR_TRANSMIT;  // 如果接收失败，返回错误信息
	}
	// 将得到的数据组合成ID
	W25Qxx_ID = (QSPI_ReceiveBuff[0] << 16) | (QSPI_ReceiveBuff[1] << 8 ) | QSPI_ReceiveBuff[2];

	return W25Qxx_ID; // 返回ID
}



/*************************************************************************************************
*	函 数 名: QSPI_W25Qxx_MemoryMappedMode
*	入口参数: 无
*	返 回 值: QSPI_W25Qxx_OK - 写使能成功，W25Qxx_ERROR_WriteEnable - 写使能失败
*	函数功能: 将QSPI设置为内存映射模式
*	说    明: 设置为内存映射模式时，只能读，不能写！！！	
**************************************************************************************************/

int8_t QSPI_W25Qxx_MemoryMappedMode(void)
{
	QSPI_CommandTypeDef      s_command;				 // QSPI传输配置
	QSPI_MemoryMappedTypeDef s_mem_mapped_cfg;	 // 内存映射访问参数

	s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    		// 1线指令模式
	s_command.AddressSize       = QSPI_ADDRESS_24_BITS;            // 24位地址
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  		// 无交替字节 
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;     		// 禁止DDR模式
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY; 		// DDR模式中数据延迟，这里用不到
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;			// 每次传输数据都发送指令	
	s_command.AddressMode 		 = QSPI_ADDRESS_4_LINES; 				// 4线地址模式
	s_command.DataMode    		 = QSPI_DATA_4_LINES;    				// 4线数据模式
	s_command.DummyCycles 		 = 6;                    				// 空周期个数
	s_command.Instruction 		 = W25Qxx_CMD_FastReadQuad_IO; 		// 1-4-4模式下(1线指令4线地址4线数据)，快速读取指令
	
	s_mem_mapped_cfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE; // 禁用超时计数器, nCS 保持激活状态
	s_mem_mapped_cfg.TimeOutPeriod     = 0;									 // 超时判断周期

	QSPI_W25Qxx_Reset();		// 复位W25Qxx
	
	if (HAL_QSPI_MemoryMapped(&hqspi, &s_command, &s_mem_mapped_cfg) != HAL_OK)	// 进行配置
	{
		return W25Qxx_ERROR_MemoryMapped; 	// 设置内存映射模式错误
	}

	return QSPI_W25Qxx_OK; // 配置成功
}

//	实验平台：反客STM32H750XBH6核心板 （型号：FK750M4-XBH6）

/********************************************************************************************************************************************************************************************************FANKE**********/

