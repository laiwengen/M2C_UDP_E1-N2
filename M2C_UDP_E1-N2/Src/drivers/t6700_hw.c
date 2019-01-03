#include "stm32f0xx_hal.h"
#include "string.h"
#include "frameworks/modbus.h"
#include "frameworks/tick.h"
#include "frameworks/t6700.h"

#define T6700_SLAVE_ADDRESS 0x15
#define MODBUS_ID 0x01
#define USART_TIMEOUT 5

#if 0

static uint16_t g_receivedValue;
static uint8_t g_received;

void received4D(modbus_listener_t*listener, uint8_t* buffer, int16_t size) {
	if (listener->functionCode == 4 && size == 7) {
		g_receivedValue = (buffer[3] << 8) | (buffer[4] << 0);
		g_received = 1;
	} else if (listener->functionCode == 5 && size == 8) {
		g_receivedValue = (buffer[4] << 8) | (buffer[5] << 0);
		g_received = 1;
	}
}

static uint8_t waitUntilReceived(uint32_t ms) {
	uint32_t s = tick_ms();
	while (tick_ms() != (s + ms)) {
		if (g_received) {
			return 1;
		}
	}
	return 0;
}

static uint8_t read(uint16_t address, uint16_t* value) {
	uint8_t buffer[10];
	g_received = 0;
	if (modbus_send(MODBUS_ID, buffer, modbus_serialize(T6700_SLAVE_ADDRESS, 4, address, 1, buffer), 7, received4D, 0)) {
		if (waitUntilReceived(USART_TIMEOUT)) {
			*value = g_receivedValue;
			return 1;
		}
	}
	return 0;
}

static uint8_t write(uint16_t address, uint16_t value, uint8_t response) {
	uint8_t buffer[10];
	g_received = 0;
	if (modbus_send(MODBUS_ID, buffer, modbus_serialize(T6700_SLAVE_ADDRESS, 5, address, 1, buffer), response?8:0, received4D, 0)) {
		if (response) {
			return 1;
		}
		if (waitUntilReceived(USART_TIMEOUT)) {
			if (value == g_receivedValue) {
				return 1;
			}
		}
	}
	return 0;
}

#endif

static uint8_t read(uint16_t address, uint16_t* value) {
	uint8_t buffer[10];
	if (modbus_send(MODBUS_ID, buffer, modbus_serialize(T6700_SLAVE_ADDRESS, 4, address, 1, buffer))){
		if (modbus_receive(MODBUS_ID, buffer, 7)) {
			if (modbus_check(buffer,7)) {
				* value = (buffer[3] << 8) | (buffer[4] << 0);
				return 1;
			}
		}
	}
	return 0;
}

static uint8_t write(uint16_t address, uint16_t value, uint8_t response) {
	uint8_t buffer[10];
	if (modbus_send(MODBUS_ID, buffer, modbus_serialize(T6700_SLAVE_ADDRESS, 5, address, 1, buffer))){
		if (!response) {
			return 1;
		}
		if (modbus_receive(MODBUS_ID, buffer, 8)) {
			if (modbus_check(buffer,8)) {
				return (value == (buffer[4] << 8) | (buffer[5] << 0));
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

void t6700_hw_enable(uint8_t enable) {
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_9,enable?GPIO_PIN_SET:GPIO_PIN_RESET);
}

void t6700_hw_init(void) {
	// modbus
}
