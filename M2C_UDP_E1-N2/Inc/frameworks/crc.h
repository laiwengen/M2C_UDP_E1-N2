#ifndef CRC_H__
#define CRC_H__

#include "stdint.h"

uint16_t crc_modbus(uint8_t* buffer, int16_t size);

#endif
