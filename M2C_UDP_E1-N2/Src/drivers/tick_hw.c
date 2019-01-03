#include "stm32f0xx_hal.h"
#include "frameworks/tick.h"
#include "frameworks/thread.h"

extern TIM_HandleTypeDef htim1;

TIM_HandleTypeDef* g_tick_htim = &htim1;

void tick_hw_init(void) {
	HAL_TIM_Base_Start_IT(g_tick_htim);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim) {
	if(htim == g_tick_htim)
	{
    tick_tick();
	}
}
