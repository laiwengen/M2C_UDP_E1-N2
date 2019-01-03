#include "stm32f0xx_hal.h"
#include "frameworks/ad.h"

ADC_TypeDef* const g_ad_hw_hadc = ADC1;
DMA_Channel_TypeDef* const g_ad_hw_hdmac = DMA1_Channel1;
DMA_TypeDef* const g_ad_hw_hdma = DMA1;

void DMA1_Channel1_IRQHandler(void) {
  uint32_t baseIsr = g_ad_hw_hdma->ISR;
  g_ad_hw_hdma->IFCR = baseIsr&0xf; // clear all;
  uint32_t channelCcr = g_ad_hw_hdmac->CCR; // for check if it enabled, really necessary?
  if ((baseIsr & (DMA_FLAG_TC1<<0)) && (channelCcr & DMA_IT_TC)) {
    ad_done();
  } else {
		__nop();
	}
}

void ad_hw_init(void) {

  NVIC_SetPriority(DMA1_Channel1_IRQn, 0);
  NVIC_EnableIRQ(DMA1_Channel1_IRQn);

 	g_ad_hw_hdmac->CCR = 0x00003580; //priority = 3, m size = 2byte, p size = 2byte, m inc = true
 	g_ad_hw_hadc->TR = 0x0FFF0000; // window for watchdog
 	g_ad_hw_hadc->CFGR1 = 0x00003000; //12bit  // continue, overwrite
 	g_ad_hw_hadc->SMPR = 0x03;  // sampling time,  total time = sampling time + converting time. fadc = 14MHz.
  //2,8,14,29,42,56,72,240
  g_ad_hw_hadc->ISR 	 = ADC_FLAG_EOC | ADC_FLAG_EOS | ADC_FLAG_OVR; // clear interrupts
	// g_ad_hw_hadc->IER 	|= ADC_IT_OVR; //  enable interrupts
  g_ad_hw_hdmac->CPAR = (uint32_t)&g_ad_hw_hadc->DR;
  g_ad_hw_hdmac->CCR |=	DMA_IT_TC;
}

uint8_t ad_hw_converting(void) {
	return g_ad_hw_hdmac->CNDTR!=0 || (g_ad_hw_hadc->CR & (ADC_CR_ADCAL)) != 0;
  // return (g_ad_hw_hadc->CR & (ADC_CR_ADSTART | ADC_CR_ADCAL)) != 0;
}

void ad_hw_calibration(void) {
	g_ad_hw_hadc->CR |= ADC_CR_ADCAL;
}

uint8_t ad_hw_config(uint8_t channel, uint8_t resolution, uint8_t sampling) {
  if (!ad_hw_converting()) {
    g_ad_hw_hadc->CHSELR = (1<<channel);
    uint32_t r = (12 - resolution) >> 1;
    r = (r & 0x03) << ADC_CFGR1_RES_Pos;
    r |= g_ad_hw_hadc->CFGR1 & ~(UINT32_C(3) << ADC_CFGR1_RES_Pos);
    g_ad_hw_hadc->CFGR1 = r;
    r = resolution > 8;
    r = ((r << 2) | (r)) << 8;
    r |= g_ad_hw_hdmac->CCR & ~(UINT32_C(0xf) << 8);
    g_ad_hw_hdmac->CCR = r;
    g_ad_hw_hadc->SMPR = sampling;
    //2,8,14,29,42,56,72,240
		return 1;
  }
	return 0;
}

void ad_hw_enable(uint8_t enable) {
  if (!enable) {
    __HAL_RCC_ADC1_CLK_ENABLE();
    if ((g_ad_hw_hadc->CR & ADC_CR_ADSTART) && !(g_ad_hw_hadc->CR & ADC_CR_ADDIS)) {
      g_ad_hw_hadc->CR |= ADC_CR_ADSTP;
      while ((g_ad_hw_hadc->CR | ADC_CR_ADSTART))
      ;
    }
    if (!(g_ad_hw_hadc->CR & ADC_CR_ADSTART) && (g_ad_hw_hadc->CR & ADC_CR_ADEN)) {
      g_ad_hw_hadc->CR |= ADC_CR_ADDIS;
    }
    __HAL_RCC_ADC1_CLK_DISABLE();
  }
}

uint8_t ad_hw_convert(void* buffer, uint16_t size) {
  if (!ad_hw_converting()) {
  	//g_ad_hw_hadc->CR |= ADC_CR_ADSTP;
  	//g_ad_hw_hadc->CR |= ADC_CR_ADDIS;
    g_ad_hw_hadc->CFGR1 |= ADC_CFGR1_DMAEN;
  	g_ad_hw_hdmac->CCR &=	~DMA_CCR_EN;
    g_ad_hw_hdmac->CNDTR = size;
    g_ad_hw_hdmac->CMAR = (uint32_t)buffer;
  	g_ad_hw_hdmac->CCR |=	DMA_CCR_EN;
    g_ad_hw_hadc->CR |= ADC_CR_ADEN; //enabel
  	g_ad_hw_hadc->CR |= ADC_CR_ADSTART;
    return 1;
  } else {
		__nop();
	}
  return 0;
}
