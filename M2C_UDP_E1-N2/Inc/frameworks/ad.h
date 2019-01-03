#ifndef AD_H__
#define AD_H__
#include "stdint.h"

void ad_done(void);
void ad_init(void);
void ad_enable(uint8_t enable);

uint32_t ad_addSingle(uint8_t channel, uint8_t resolution, uint8_t sampling);
uint32_t ad_add(uint8_t* buffer, int16_t size, uint8_t channel, uint8_t resolution, uint8_t sampling);

int16_t ad_getSingle(uint32_t id);
uint8_t* ad_peek(uint32_t id);
void ad_drop(uint32_t id);
void ad_start(void);
uint8_t ad_converting(void);

// hw functions
void ad_hw_init(void);
void ad_hw_enable(uint8_t enable);
uint8_t ad_hw_converting(void);
void ad_hw_calibration(void);
uint8_t ad_hw_config(uint8_t channel, uint8_t resolution, uint8_t sampling);
uint8_t ad_hw_convert(void* buffer, uint16_t size);

#endif
