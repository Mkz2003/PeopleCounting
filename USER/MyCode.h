#ifndef __MYCODE_H
#define __MYCODE_H

#include "main.h"
#include "cmsis_os.h"
#include "adc.h"
#include "bdma.h"
#include "dcmi.h"
#include "dma.h"
#include "dma2d.h"
#include "fatfs.h"
#include "jpeg.h"
#include "libjpeg.h"
#include "mdma.h"
#include "quadspi.h"
#include "rtc.h"
#include "spi.h"
#include "usart.h"
#include "usb_device.h"
#include "gpio.h"
#include "fmc.h"
#include "jpeg_utils.h"

#include <stdio.h>

#include "usbd_cdc_if.h"
#include "lcd_spi_200.h"
#include "dcmi_ov5640.h"
#include "stm32ipl.h"
#include "MySTM32IPL.h"
#include "DMA2DProcess.h"
#include "qspi_w25q64.h"
#include "JPEGEncode.h"
#include "MyByteTrack.h"

void Setup(void);
void CDC_printf(const char* fmt, ...);


#endif /* __MYCODE_H */
