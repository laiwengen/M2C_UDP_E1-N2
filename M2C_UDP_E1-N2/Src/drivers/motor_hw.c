#include "stm32f0xx_hal.h"

static TIM_TypeDef* const g_timOut = TIM3;
static uint32_t g_count;
static uint32_t g_base;

#define PIN_INDEX 4
#define PIN_VALUE (1<<PIN_INDEX)
#define PORT 0 /* for PORTA*/

void motor_hw_setPwm(uint32_t pwm) {
  g_timOut->CCR1 = pwm;
}

int32_t motor_hw_peekCounter(void) {
  return g_count - g_base;
}

uint32_t motor_hw_getCounter(void) {
  uint32_t shadow = g_count;
  uint32_t toRet = shadow - g_base;
  g_base = shadow;
  return toRet;
}

void motor_hw_init(void) {

  RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
  RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

  GPIOA->MODER = (GPIOA->MODER & ~(3<<(6*2)))|(2<<(6*2));	//pa6 alternate mode
  GPIOA->AFR[0]= (GPIOA->AFR[0] & ~(0xf<<(6*4)))|(1<<(6*4)); // pa6 afr 1, TIM3_CH1

  g_timOut->PSC = 1; //prescare  //24M
  g_timOut->ARR = 999;  // 24k Hz
  g_timOut->CCR1 = 100;  // 10%
  g_timOut->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1PE;
  g_timOut->CCER |= TIM_CCER_CC1E;
  g_timOut->BDTR |= TIM_BDTR_MOE;

  GPIOA->MODER = (GPIOA->MODER & ~(3<<(4*2))) | (0<<(4*2)); // pa4 input mode
  GPIOA->PUPDR = (GPIOA->PUPDR & ~(3<<(4*2))) | (1<<(4*2)); // pa4 pull up

  /* Enable SYSCFG Clock */
  __HAL_RCC_SYSCFG_CLK_ENABLE();
  uint32_t temp;
  temp = SYSCFG->EXTICR[PIN_INDEX>>2]; // pin4
  temp &= ~(0xf << ((PIN_INDEX & 0x03) * 4)); // pin4
  temp |= (PORT << ((PIN_INDEX & 0x03) * 4)); // pa
  SYSCFG->EXTICR[PIN_INDEX>>2] = temp; // pin4
  /* Clear EXTI line configuration */
  EXTI->IMR &= ~PIN_VALUE; // pin4;
  EXTI->EMR &= ~PIN_VALUE; // pin4;
  EXTI->RTSR |= PIN_VALUE; // rising;
  EXTI->FTSR |= PIN_VALUE; // falling;
	HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0x00, 0); //PIN_INDEX
	HAL_NVIC_EnableIRQ(EXTI4_15_IRQn); //PIN_INDEX
}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  if (GPIO_Pin == PIN_VALUE) {
    g_count ++;
  }
}
void motor_hw_enable(uint8_t enable) {
  if (enable) {
    g_timOut->CR1 |= TIM_CR1_CEN;
    g_timOut->EGR |= TIM_EGR_UG;
    EXTI->IMR |= PIN_VALUE;
  } else {
    g_timOut->CR1 &= ~TIM_CR1_CEN;
    EXTI->IMR &= ~PIN_VALUE;
  }
}
