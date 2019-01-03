#ifndef DS18B20_H__
#define DS18B20_H__

#include "stdint.h"

uint16_t ds18b20_get(uint32_t id);
void ds18b20_init(uint32_t id);
void ds18b20_enable(uint32_t id, uint8_t enable);

#endif
