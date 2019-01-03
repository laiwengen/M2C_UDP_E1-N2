#include "stm32f0xx_hal.h"
#include "frameworks/number.h"
#include "frameworks/ad.h"
#include "frameworks/battery.h"
#include "frameworks/average.h"

#define AD_CHANNEL ADC_CHANNEL_5
static uint32_t g_id;

uint8_t battery_hw_isCharging(void) {
  return HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_7) != 0;
}

const static uint16_t g_xs[] = {2240, 2320, 2400, 2480, 2580};
const static uint16_t g_ys[] = {0, 25, 50, 75, 100};
const static average_tab_t g_tab= {g_xs, g_ys, 5, 0, 0};

int16_t battery_hw_getPercent(void) {
  int32_t ad = ad_getSingle(g_id);
  if (ad > 0) {
		int32_t offset = max(min((2590 - ad)*2, 100),20);
		int32_t x = ad + (battery_hw_isCharging()?-offset:0);
    int16_t percent = average_tab_correct(&g_tab, x);
    percent = max(percent,0);
//    percent = min(percent,100);
    return percent;
  } else {
    if (!ad_converting()) {
      ad_start();
    }
  }
  return -1;
}


void battery_hw_init(void) {
  g_id = ad_addSingle(AD_CHANNEL,12,7);
  ad_start();
}
