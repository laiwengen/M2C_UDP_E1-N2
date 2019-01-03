#include "stm32f0xx_hal.h"
#include "frameworks/schedule.h"
#include "frameworks/number.h"
#include "frameworks/esp8266.h"

#define ESP8266_HW_POWERON_SCHEDULE_ID 0x42fd8266
#define ESP8266_HW_POWEROFF_SCHEDULE_ID 0x42ff8266
__IO int8_t g_esp8266_hw_scheduleLeft = 0;
extern esp8266_t* g_esp8266_main;

static DMA_TypeDef* const g_hdmaBase = DMA1;
static DMA_Channel_TypeDef* const g_hdmaIn = DMA1_Channel3;
static DMA_Channel_TypeDef* const g_hdmaOut = DMA1_Channel2;
static USART_TypeDef* const g_huart = USART1;

static uint8_t g_transmiting = 0;
static uint8_t g_esp8266_PowerOn = 0;
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
		
		reloadOutDma(g_esp8266_main->outStream->circularBuffer);
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

void flushEsp8266TransmitBuffer(void)
{
		g_transmiting = 0;
		reloadOutDma(g_esp8266_main->outStream->circularBuffer);
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
//	{
//		uint32_t cselr = g_hdmaBase->CSELR;
//		cselr &= ~(0xFU << 8);
//		cselr |= 0x09 << 8;
//		g_hdmaBase->CSELR = cselr;
//	}
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
//	{
//		uint32_t cselr = g_hdmaBase->CSELR;
//		cselr &= ~(0xFU << 4);
//		cselr |= 0x09 << 4;
//		g_hdmaBase->CSELR = cselr;
//	}
	hdma->CNDTR = stream->circularBuffer->size;
  hdma->CPAR = (uint32_t)&g_huart->TDR;
	//
//	hdma->CMAR = (uint32_t)stream->circularBuffer->buffer;
//	hdma->CCR |=	DMA_IT_TC;
//	hdma->CCR |=	DMA_CCR_EN;
	//
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
uint8_t esp8266_hw_isPowerOn(void) {
//  return HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0)?1:0;
	 return g_esp8266_PowerOn;
}

void esp8266_hw_init(void) {
	esp8266_t* esp8266 = g_esp8266_main;
	HAL_NVIC_SetPriority(DMA1_Channel2_3_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);
//	HAL_GPIO_WritePin(WIFI_CH_PD_GPIO_Port,WIFI_CH_PD_Pin,GPIO_PIN_SET);
	initInStream(esp8266->inStream);
	initOutStream(esp8266->outStream);
}


//void esp8266_hw_power4D(void** params) {
//  if (g_esp8266_hw_scheduleLeft > 0) {
//  	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,(g_esp8266_hw_scheduleLeft&1)?GPIO_PIN_RESET:GPIO_PIN_SET);
//    g_esp8266_hw_scheduleLeft --;
//    if (g_esp8266_hw_scheduleLeft > 0) {
//      schedule_once(ESP8266_HW_SCHEDULE_ID,esp8266_hw_power4D,1200,params);
//    }
//  }
//}



//uint8_t esp8266_hw_isIniting(void) {
//  return schedule_exists(esp8266_HW_SCHEDULE_ID);
//}


void esp8266_hw_powerOff(void** params) {
//	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_RESET);
//	if(esp8266_hw_isPowerOn()) {
//    if (!esp8266_hw_isIniting()) {
//      g_esp8266_hw_scheduleLeft += 2;
//	    schedule_once(ESP8266_HW_SCHEDULE_ID,esp8266_hw_power4D,1200,0);
//    }
//	}

  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_11,GPIO_PIN_RESET);
	g_esp8266_PowerOn = 0;
}
void esp8266_hw_powerOn(void** params) {
//	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_SET);
//	HAL_GPIO_WritePin(WIFI_RESET_GPIO_Port,WIFI_RESET_Pin,GPIO_PIN_SET);
//	HAL_GPIO_WritePin(WIFI_POWE_GPIO_Port,WIFI_POWE_Pin,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_11,GPIO_PIN_SET);
	
	g_esp8266_PowerOn = 1;
}

void esp8266_hw_reset(void) {
//	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_SET);////////////////没有
	//if (esp8266_hw_isPowerOn())
	{
		
		schedule_once(ESP8266_HW_POWERON_SCHEDULE_ID,esp8266_hw_powerOff,10,0);
		schedule_once(ESP8266_HW_POWEROFF_SCHEDULE_ID,esp8266_hw_powerOn,200,0);
	}

}




//static void initInStream(stream_t* stream) {
//	initInStreamDma(stream, g_uartPollUart, g_hdmaIn);
//}

//static void initOutStream(stream_t* stream) {
//	if (stream->circularBuffer) {
////		initOutStreamDma(stream, g_huart, g_hdmaOut);
////		stream->flushFunction = circularBufferFlushFunction;
//	} else {
//		stream->flushFunction = noBufferFlush;
//	}
//}

//void esp8266_hw_init(esp8266_t* esp8266) {
//	g_esp8266 = esp8266;
//	initInStream(esp8266->inStream);
//	initOutStream(esp8266->outStream);
//}
//void esp8266_hw_init(esp8266_t* esp8266)
//{
//	
//}

