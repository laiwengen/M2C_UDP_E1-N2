#include "stm32f0xx_hal.h"
#include "string.h"
#include "frameworks/t6700.h"

#define T6700_I2C_ADDRESS 0x2a
#define I2C_TIMEOUT 5

extern I2C_HandleTypeDef hi2c1;
static I2C_HandleTypeDef* const g_hi2c = &hi2c1; // TODO use reg instead of HAL handle

static uint8_t g_readBytes[] = {0x04, 0, 0, 0x00, 0x01};

static uint8_t read(uint16_t address, uint16_t* value) {
	uint8_t buffer[4] = {0};
  g_readBytes[1] = address>>8;
  g_readBytes[2] = address&0xff;
	if (HAL_OK == HAL_I2C_Master_Transmit(g_hi2c,T6700_I2C_ADDRESS,g_readBytes,sizeof(g_readBytes),I2C_TIMEOUT)){
    if (HAL_OK == HAL_I2C_Master_Receive(g_hi2c,T6700_I2C_ADDRESS,buffer,sizeof(buffer),I2C_TIMEOUT)){
      if (buffer[0] == 0x04 && buffer[1] == 0x02) {
        *value = (buffer[2]<<8)|buffer[3];
        return 1;
      }
    }
  }
	return 0;
}

static uint8_t write(uint16_t address, uint16_t value, uint8_t response) {
  uint8_t bytes[] = {0x05, address>>8, address&0xff, value>>8, value&0xff};
  uint8_t buffer[5] = {0};
	if (HAL_OK == HAL_I2C_Master_Transmit(g_hi2c,T6700_I2C_ADDRESS,bytes,sizeof(bytes),I2C_TIMEOUT)){
    if (!response) {
      return 1;
    }
    if (HAL_OK == HAL_I2C_Master_Receive(g_hi2c,T6700_I2C_ADDRESS,buffer,sizeof(buffer),I2C_TIMEOUT)){
      if (memcmp(bytes,buffer,sizeof(bytes)) == 0) {
        return 1;
      }
    }
  }
	return 0;
}

int16_t t6700_hw_gasPpm(void) {
  uint16_t value;
  if (read(0x138B, &value)) {
    return (int16_t) value;
  }
  return -1;
}

uint16_t t6700_hw_status(void) {
  uint16_t value;
  if (read(0x138A, &value)) {
    return value;
  }
  return 0;
}

uint8_t t6700_hw_abcLogic(uint8_t enable) {
  return write(0x03ee,enable?0xff00:0x0000,0);
}

uint8_t t6700_hw_calibrate(uint8_t enable) {
  return write(0x03ec,enable?0xff00:0x0000,1);
}

void t6700_hw_init(void) {
	// i2c's?
}

void t6700_hw_enable(uint8_t enable) {
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_9,enable?GPIO_PIN_SET:GPIO_PIN_RESET);
}
