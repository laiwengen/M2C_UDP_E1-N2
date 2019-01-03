#include "stm32f0xx_hal.h"
// #include "frameworks/number.h"
#include "frameworks/list.h"
#include "frameworks/ad.h"
#include "frameworks/dust.h"

#define AD_CHANNEL_DC 10
#define AD_CHANNEL_AC 11

// #define fortas(type, array, size) for(int16_t i = 0, type v = array[0]; i < size; i++, v = array[i])

int16_t* dust_hw_getAcData(dust_t* dust) {
  fortas(uint8_t, dust->ac.ids, 2) {
    int16_t* address = (int16_t*)ad_peek(v);
    if (address) {
      return address;
    }
  }
  return 0;
}

void dust_hw_dropAcData(dust_t* dust, int16_t* a) {
  fortas(uint8_t, dust->ac.ids, 2) {
    int16_t* address = (int16_t*)ad_peek(v);
    if (address == a) {
      ad_drop(v);
      return;
    }
  }
}

void dust_hw_fan(uint8_t enable) {
  HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8,(enable)?GPIO_PIN_SET:GPIO_PIN_RESET);
}

void dust_hw_laser(uint8_t enable) {
  HAL_GPIO_WritePin(GPIOC,GPIO_PIN_15,(enable)?GPIO_PIN_SET:GPIO_PIN_RESET);
}

void dust_hw_init(dust_t* dust) {
  fors(2) {
    dust->ac.ids[i] = ad_add(dust->ac.buffers[i],dust->ac.size,AD_CHANNEL_AC,12,3);
  }
  ad_start();
}
