#include "frameworks/console.h"
#include "frameworks/number.h"
#include "stm32f0xx_hal.h"

static DMA_TypeDef* const g_hdmaBase = DMA1;
static DMA_Channel_TypeDef* const g_hdmaIn = DMA1_Channel5;
static DMA_Channel_TypeDef* const g_hdmaOut = DMA1_Channel4;
static USART_TypeDef* const g_huart = USART3;

static console_t* g_console;
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


void DMA1_Channel4_5_IRQHandler(void) {
	uint32_t isr = g_hdmaBase->ISR;
	if (isr & (DMA_FLAG_TC4 | DMA_FLAG_TE4)) {
		if (isr & DMA_FLAG_TE4) {
			g_hdmaBase->IFCR = DMA_FLAG_GL4;
		} else {
			g_hdmaBase->IFCR = DMA_FLAG_TC4;
		}
  	g_hdmaOut->CCR &= ~(DMA_IT_TC | DMA_IT_TE);
		g_transmiting = 0;
		reloadOutDma(g_console->outStream->circularBuffer);
		// t tc / e?
	}
	if (isr & DMA_FLAG_TC5) {
		g_hdmaBase->IFCR = DMA_FLAG_TC5;
		// r tc
	}
	if (isr & DMA_FLAG_TE5) {
		g_hdmaBase->IFCR = DMA_FLAG_GL5;
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
		cselr &= ~(0xFU << 16);
		cselr |= 0x0a << 16;
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
		cselr &= ~(0xFU << 12);
		cselr |= 0x0a << 12;
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

void console_hw_init(console_t* console) {
	g_console = console;
	initInStream(console->inStream);
	initOutStream(console->outStream);
}
