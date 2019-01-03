#include "stm32f0xx_hal.h"
#include "frameworks/modbus.h"
#include "frameworks/number.h"
#include "frameworks/schedule.h"

static USART_TypeDef* const g_huart = USART3;

static DMA_TypeDef* const g_hdmaBase = DMA1;
static DMA_Channel_TypeDef* const g_hdmaIn = DMA1_Channel3;
static DMA_Channel_TypeDef* const g_hdmaOut = DMA1_Channel2;
static int32_t const g_dmaIndexIn = 2;
static int32_t const g_dmaIndexOut = 1;

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
modbusSlave_t* g_modbus_main = 0;
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
		reloadOutDma(g_modbus_main->outStream->circularBuffer);
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


void USART3_6_IRQHandler(void) {
  uint32_t isr = g_huart->ISR;
  g_huart->ICR = isr;
	if (!g_transmiting && (isr | USART_ISR_TC)) {
			modbus_hw_postSend();
	}
}


static int16_t getIndexIn(circularBuffer_t* cb) {
	return cb->size - g_hdmaIn->CNDTR;
}

static int16_t noBufferFlushFunction(stream_t* stream, uint8_t byte) {
  g_huart->TDR = (byte & (uint8_t)0xFF);
  uint32_t times = 160000;
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
		cselr &= ~(0xFU << (g_dmaIndexIn*4));
		cselr |= 0x0a << (g_dmaIndexIn*4);
		g_hdmaBase->CSELR = cselr;
	}
  hdma->CNDTR = stream->circularBuffer->size;
  hdma->CPAR = (uint32_t)&g_huart->RDR;
  hdma->CMAR = (uint32_t)stream->circularBuffer->buffer;
	hdma->CCR |=	DMA_IT_TC;
	hdma->CCR |=	DMA_CCR_EN;

	g_huart->CR3 |= USART_CR3_DMAR;
	g_huart->CR3 |= USART_CR3_EIE;

	stream->circularBuffer->writeIndex.function = getIndexIn;
}

static void initOutStreamDma(stream_t* stream, USART_TypeDef* g_huart, DMA_Channel_TypeDef* hdma){
	hdma->CCR &= ~DMA_CCR_EN;
	hdma->CCR = 0x2090;
	{
		uint32_t cselr = g_hdmaBase->CSELR;
		cselr &= ~(0xFU << (g_dmaIndexOut*4));
		cselr |= 0x0a << (g_dmaIndexOut*4);
		g_hdmaBase->CSELR = cselr;
	}
  hdma->CPAR = (uint32_t)&g_huart->TDR;

	g_huart->CR3 |= USART_CR3_DMAT;
	g_huart->CR3 |= USART_CR3_EIE;

	g_huart->CR1 |= USART_CR1_TCIE;
}

#if 0
  static circularBuffer_t * inStreamCB;
  void USART1_IRQHandler(void) {
    uint32_t isr = g_huart->ISR;
		g_huart->ICR = isr;
    if (isr & USART_ISR_RXNE) {
      uint8_t value = g_huart->RDR;
      cb_write(inStreamCB, &value, 1);
    }
  }
  static void initInStream(stream_t* stream) {
    inStreamCB = stream->circularBuffer;
    g_huart->CR1 |= USART_CR1_RXNEIE;
  }
#endif

uint8_t modbus_hw_preSend(void) {
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET);
	return 1;
}
uint8_t modbus_hw_postSend(void) {
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET);
	return 1;
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

void modbus_hw_initSlave(modbusSlave_t* modbus) {
	g_modbus_main = modbus;
	initInStream(modbus->inStream);
	initOutStream(modbus->outStream);
}
