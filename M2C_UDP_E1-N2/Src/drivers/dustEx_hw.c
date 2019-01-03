#include "stm32f0xx_hal.h"
// #include "frameworks/schedule.h"
#include "frameworks/tick.h"
#include "frameworks/dustEx.h"

static USART_TypeDef* g_huart = 0;

static void updateHuartByDustEx(dustEx_t* dustEx) {
	g_huart = dustEx->id == 1?USART2:USART1;
}

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

#define getlast(n) (head[(hi-(n)-1) & 0x03])
#define pushlast(v) (head[(hi++) & 0x03]) = (v)

static uint8_t receive(uint8_t* byte, uint32_t wait) {
	uint32_t start = tick_ms();
  while((g_huart->ISR & UART_FLAG_RXNE)==0) {
		uint32_t now = tick_ms();
		if ((int32_t)(now-start-wait)>0) {
			return 0;
		}
	}
  *byte = (g_huart->RDR & (uint8_t)0xFF);
  return 1;
}

uint8_t dustEx_hw_receive(dustEx_t* dustEx, uint8_t* buffer, int16_t size) {
	updateHuartByDustEx(dustEx);
	uint8_t head[4];
	int16_t hi = 0;
	uint8_t matched = 0;
  fors(size) {
    uint8_t value;
    if (!receive(&value, 50)) {
      return 0;
    }
		if (matched) {
    	buffer[i] = value;
		} else {
			pushlast(value);
			i = 0;
			if (getlast(0) == 0 && getlast(1) == 0x34 && getlast(2) == 0xa5 && getlast(3) == 0x5a) {
				matched = 1;
				foris(j, 4) {
					buffer[3-j] = getlast(j);
				}
				i = 3;
			}
		}
  }
  return 1;
}

uint8_t dustEx_hw_send(dustEx_t* dustEx, uint8_t* buffer, int16_t size) {
	updateHuartByDustEx(dustEx);
  fortas(uint8_t, buffer, size) {
    if (!send(v, 5)) {
      return 0;
    }
  }
  return 1;
}

void dustEx_hw_init(dustEx_t* dustEx) {
	USART1->CR3 |= USART_CR3_EIE;
	USART2->CR3 |= USART_CR3_EIE;
}

void dustEx_hw_enable(dustEx_t* dustEx, uint8_t enable) {
	if (dustEx->id == 1) {
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, enable?GPIO_PIN_SET:GPIO_PIN_RESET);
	} else if (dustEx->id == 2) {
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, enable?GPIO_PIN_SET:GPIO_PIN_RESET);
	} else if (dustEx->id == 3) {
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, enable?GPIO_PIN_SET:GPIO_PIN_RESET);
	}
}
