#include "stm32f0xx_hal.h"
// #include "frameworks/schedule.h"
#include "frameworks/tick.h"
#include "frameworks/modbus.h"

static USART_TypeDef* const g_huart = USART1;

static uint8_t send(uint8_t byte, uint32_t wait) {
	uint32_t start = tick_ms();
  g_huart->TDR = (byte & (uint8_t)0xFF);
  while((g_huart->ISR & UART_FLAG_TC)==0) {
		if ((int32_t)(tick_ms()-start-wait)>0) {
			return 0;
		}
	}
  return 1;
}

static uint8_t receive(uint8_t* byte, uint32_t wait) {
	uint32_t start = tick_ms();
  while((g_huart->ISR & UART_FLAG_RXNE)==0) {
		if ((int32_t)(tick_ms()-start-wait)>0) {
			return 0;
		}
	}
  *byte = (g_huart->RDR & (uint8_t)0xFF);
  return 1;
}

uint8_t modbus_hw_receive(modbusMaster_t* modbus, uint8_t* buffer, int16_t size) {
  fors(size) {
    uint8_t value;
    if (!receive(&value, 5)) {
      return 0;
    }
    buffer[i] = value;
  }
  return 1;
}

uint8_t modbus_hw_send(modbusMaster_t* modbus, uint8_t* buffer, int16_t size) {
  fortas(uint8_t, buffer, size) {
    if (!send(v, 5)) {
      return 0;
    }
  }
  return 1;
}

void modbus_hw_initMaster(modbusMaster_t* modbus) {

}
