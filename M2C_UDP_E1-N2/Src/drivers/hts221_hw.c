#include "stm32f0xx_hal.h"
#include "string.h"
#include "frameworks/t6700.h"

#define I2C_ADDRESS 0xbe
#define I2C_TIMEOUT 5
// static I2C_TypeDef* const g_hi2c = I2C1;

extern I2C_HandleTypeDef hi2c1;
static I2C_HandleTypeDef* const g_hi2c = &hi2c1; // TODO use reg instead of HAL handle

static uint8_t read(uint8_t address, void* buffer, int16_t size) {
  if (HAL_I2C_Mem_Read(g_hi2c, I2C_ADDRESS, address|0x80, I2C_MEMADD_SIZE_8BIT,buffer,size, I2C_TIMEOUT)==HAL_OK) {
    return 1;
  }
	return 0;
}

static uint8_t write(uint8_t address, uint8_t value) {
	if (HAL_I2C_Mem_Write(g_hi2c,I2C_ADDRESS,address,I2C_MEMADD_SIZE_8BIT, &value, 1, I2C_TIMEOUT)==HAL_OK) {
		uint8_t read;
		if (HAL_I2C_Mem_Read(g_hi2c, I2C_ADDRESS, address, I2C_MEMADD_SIZE_8BIT,&read,1, I2C_TIMEOUT)==HAL_OK) {
			if (read == value) {
				return 1;
			}
		}
	}
	return 0;
}

void hts221_hw_config(int8_t address, uint8_t value) {
  // for (int16_t i = 0; i < 3; i++) {
  //   write(0x20, (value >> (i*8)) & 0xff);
  // }
  write(address, value);
}

void hts221_hw_start(void) {
  write(0x21, 1);
}

uint8_t hts221_hw_status(void) {
  uint8_t value = 0xff;
  if (read(0x27, &value, 1)) {
    return value;
  }
  return 0;
}

uint8_t hts221_hw_fetchData(int16_t* buffer) {
  if (read(0x28, buffer, 0x04)) {
    return 1;
  }
  return 0;
}

uint8_t hts221_hw_fetchCalibrate(uint8_t* buffer) {
  if (read(0x30, buffer, 0x10)) {
    return 1;
  }
  return 0;
}

void hts221_hw_init(void) {
}
