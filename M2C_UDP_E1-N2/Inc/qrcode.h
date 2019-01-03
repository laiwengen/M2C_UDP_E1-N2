#ifndef QRCODE_H__
#define QRCODE_H__
#include "stdint.h"

uint8_t* qrcode_convert(uint8_t* buffer, int16_t size);
uint8_t qrcode_get(uint8_t* buffer, int16_t x, int16_t y);

#endif
