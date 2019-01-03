#include "stm32f0xx_hal.h"
#include "string.h"
#include "frameworks/sht3x.h"

#define I2C_ADDRESS 0x88
#define I2C_TIMEOUT 5
// static I2C_TypeDef* const g_hi2c = I2C1;

extern I2C_HandleTypeDef hi2c1;
static I2C_HandleTypeDef* const g_hi2c = &hi2c1; // TODO use reg instead of HAL handle

static uint8_t read(void* buffer, int16_t size) {
  if (HAL_I2C_Mem_Read(g_hi2c, I2C_ADDRESS, 0xe000, I2C_MEMADD_SIZE_16BIT,buffer,size, I2C_TIMEOUT)==HAL_OK) {
    return 1;
  }
	return 0;
}

static uint8_t write(uint16_t value) {
  uint8_t buffer[] = {(value >>8)&0xff, value&0xff};
  return HAL_I2C_Master_Transmit(g_hi2c,I2C_ADDRESS,buffer,2,I2C_TIMEOUT) == HAL_OK;
}

void sht3x_hw_start(void) {
  write(0x2236);
}

uint8_t sht3x_hw_fetchData(uint8_t* buffer) {
  if (read(buffer, 0x06)) {
    return 1;
  }
  return 0;
}

void sht3x_hw_init(void) {
}
