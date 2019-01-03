#ifndef TICK_H__
#define TICK_H__
#include "stm32f0xx_hal.h"
#include "frameworks/thread.h"

static inline uint32_t tick_ms()
{
	return HAL_GetTick();
}


void tick_init(void);
void tick_add(thread_executeFunction_t function, uint32_t interval, void** params);
void tick_start(void);
void tick_stop(void);
void tick_hw_init(void);
void tick_tick(void);

#endif
