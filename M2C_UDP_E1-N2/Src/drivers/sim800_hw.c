
#include "stm32f0xx_hal.h"
#include "frameworks/schedule.h"
#include "frameworks/number.h"
#include "frameworks/sim800.h"

#define SIM800_HW_SCHEDULE_ID 0x42fd534a

__IO int8_t g_sim800_hw_scheduleLeft = 0;
extern sim800_t* g_sim800_main;

static DMA_TypeDef* const g_hdmaBase = DMA1;
static DMA_Channel_TypeDef* const g_hdmaIn = DMA1_Channel3;
static DMA_Channel_TypeDef* const g_hdmaOut = DMA1_Channel2;
static USART_TypeDef* const g_huart = USART2;

static uint8_t g_transmiting = 0;

static int16_t reloadOutDma(circularBuffer_t* cb) {
	if (g_transmiting) {
		return 0;
	}
	int16_t size = cb_size(cb);
	if (size == 0) {
		return 0;
	}
	g_transmiting = 1;
	int16_t readIndex = cb->readIndex.value;
	int16_t toEndSize = cb->size - readIndex;
	size = min(size, toEndSize);
	uint8_t* ptr = cb->buffer + readIndex;
	cb_drop(cb, size);

	g_hdmaOut->CCR &=  ~DMA_CCR_EN;
  g_hdmaOut->CNDTR = size;
  g_hdmaOut->CMAR = (uint32_t)ptr;
	g_hdmaOut->CCR |=	DMA_IT_TC;
	g_hdmaOut->CCR |=	DMA_CCR_EN;
	return size;
}


void DMA1_Channel2_3_IRQHandler(void) {
	uint32_t isr = g_hdmaBase->ISR;
	if (isr & (DMA_FLAG_TC2 | DMA_FLAG_TE2)) {
		if (isr & DMA_FLAG_TE2) {
			g_hdmaBase->IFCR = DMA_FLAG_GL2;
		} else {
			g_hdmaBase->IFCR = DMA_FLAG_TC2;
		}
  	g_hdmaOut->CCR &= ~(DMA_IT_TC | DMA_IT_TE);
		g_transmiting = 0;
		reloadOutDma(g_sim800_main->outStream->circularBuffer);
		// t tc / e?
	}
	if (isr & DMA_FLAG_TC3) {
		g_hdmaBase->IFCR = DMA_FLAG_TC3;
		// r tc
	}
	if (isr & DMA_FLAG_TE3) {
		g_hdmaBase->IFCR = DMA_FLAG_GL3;
	}
}

static int16_t getIndex1(circularBuffer_t* cb) {
	return cb->size - g_hdmaIn->CNDTR;
}

static int16_t noBufferFlushFunction(stream_t* stream, uint8_t byte) {
	g_huart->TDR = (byte & (uint8_t)0xFF);
	uint32_t times = 16000;
	while((g_huart->ISR & UART_FLAG_TC)==0 && --times)
		;
	return times != 0 ? 1 : 0;
}

static int16_t circularBufferFlushFunction(stream_t* stream, uint8_t ignore) {
	return reloadOutDma(stream->circularBuffer);
}
static void initInStreamDma(stream_t* stream, USART_TypeDef* g_huart, DMA_Channel_TypeDef* hdma){
	hdma->CCR &= ~DMA_CCR_EN;
	hdma->CCR = 0x20A0;
	{
		uint32_t cselr = g_hdmaBase->CSELR;
		cselr &= ~(0xFU << 8);
		cselr |= 0x09 << 8;
		g_hdmaBase->CSELR = cselr;
	}
  hdma->CNDTR = stream->circularBuffer->size;
  hdma->CPAR = (uint32_t)&g_huart->RDR;
  hdma->CMAR = (uint32_t)stream->circularBuffer->buffer;
	hdma->CCR |=	DMA_IT_TC;
	hdma->CCR |=	DMA_CCR_EN;

	g_huart->CR3 |= USART_CR3_DMAR;
	g_huart->CR3 |= USART_CR3_EIE;

	stream->circularBuffer->writeIndex.function = getIndex1;
}

static void initOutStreamDma(stream_t* stream, USART_TypeDef* g_huart, DMA_Channel_TypeDef* hdma){
	hdma->CCR &= ~DMA_CCR_EN;
	hdma->CCR = 0x2090;
	{
		uint32_t cselr = g_hdmaBase->CSELR;
		cselr &= ~(0xFU << 4);
		cselr |= 0x09 << 4;
		g_hdmaBase->CSELR = cselr;
	}
  hdma->CPAR = (uint32_t)&g_huart->TDR;

	g_huart->CR3 |= USART_CR3_DMAT;
	g_huart->CR3 |= USART_CR3_EIE;
}

static void initInStream(stream_t* stream) {
	initInStreamDma(stream, g_huart, g_hdmaIn);
}

static void initOutStream(stream_t* stream) {
	if (stream->circularBuffer) {
		initOutStreamDma(stream, g_huart, g_hdmaOut);
		stream->flushFunction = circularBufferFlushFunction;
	} else {
		stream->flushFunction = noBufferFlushFunction;
	}
}

void sim800_hw_init(void) {
	sim800_t* sim800 = g_sim800_main;
	initInStream(sim800->inStream);
	initOutStream(sim800->outStream);
}


void sim800_hw_power4D(void** params) {
  if (g_sim800_hw_scheduleLeft > 0) {
  	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,(g_sim800_hw_scheduleLeft&1)?GPIO_PIN_RESET:GPIO_PIN_SET);
    g_sim800_hw_scheduleLeft --;
    if (g_sim800_hw_scheduleLeft > 0) {
      schedule_once(SIM800_HW_SCHEDULE_ID,sim800_hw_power4D,1200,params);
    }
  }
}

uint8_t sim800_hw_isPowerOn(void) {
  return HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0)?1:0;
}

uint8_t sim800_hw_isIniting(void) {
  return schedule_exists(SIM800_HW_SCHEDULE_ID);
}



void sim800_hw_powerOn(void) {
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_SET);
  if (!sim800_hw_isIniting()) {
  	if(!sim800_hw_isPowerOn()) {
      g_sim800_hw_scheduleLeft += 2;
	    schedule_once(SIM800_HW_SCHEDULE_ID,sim800_hw_power4D,1200,0);
    }
  }
}

void sim800_hw_reset(void) {
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_SET);
  if (!sim800_hw_isIniting()) {
  	if(sim800_hw_isPowerOn()) {
      g_sim800_hw_scheduleLeft += 4;
  	} else {
      g_sim800_hw_scheduleLeft += 2;
    }
    schedule_once(SIM800_HW_SCHEDULE_ID,sim800_hw_power4D,1,0);
  }
}

void sim800_hw_powerOff() {
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_RESET);
	if(sim800_hw_isPowerOn()) {
    if (!sim800_hw_isIniting()) {
      g_sim800_hw_scheduleLeft += 2;
	    schedule_once(SIM800_HW_SCHEDULE_ID,sim800_hw_power4D,1200,0);
    }
	}
}
