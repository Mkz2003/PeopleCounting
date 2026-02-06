#ifndef __JPEGENCODE_H
#define __JPEGENCODE_H

#include <stdint.h>

extern uint8_t jpeg_buffer[];

void JPEGEncodeInit();
uint32_t JPEGEncode(uint8_t* srcImage, uint32_t srcSize);

#endif // __JPEGENCODE_H