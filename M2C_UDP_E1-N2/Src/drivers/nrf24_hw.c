#include "frameworks/nrf24.h"

static SPI_TypeDef const* const g_spi = SPI1;
static TIM_TypeDef const* const g_tim = TIM6;

static uint8_t g_timeout;
static nrf24_t* g_nrf24;

static void csn(uint8_t set) {
  if (set) {
    GPIOA->BSRR = 1<<(5);
  } else {
    GPIOA->BRR = 1<<(5);
  }
}

static uint8_t communicate (uint8_t data) {
  if (g_spi->SR & SPI_SR_TXE) {
   *(uint8_t *)&(g_spi->DR) = data;
  }
  while (!(g_spi->SR & SPI_SR_RXNE));
  return (uint8_t)g_spi->DR;
}

uint8_t nrf24_hw_write(nrf24_t* nrf24, uint8_t command, uint8_t const* data, int8_t size) {
  csn(0);
  uint8_t status = communicate(command);
  fortas(uint8_t, data, size) {
    communicate(v);
  }
  csn(1);
  return status;
}

uint8_t nrf24_hw_read(nrf24_t* nrf24, uint8_t command, uint8_t* data, int8_t size) {
  csn(0);
  uint8_t status = communicate(command);
  fors(size) {
    data[i] = communicate(0);
  }
  csn(1);
  return status;
}

void TIM6_IRQHandler(void) {
  g_tim->CR1 &= ~TIM_CR1_CEN;
  g_tim->SR = TIM_SR_UIF;
  nrf24_interrputed(g_nrf24, g_timeout?0x01:0x02);
}

void nrf24_hw_setTimer(nrf24_t* nrf24, uint32_t us, uint8_t timeout) {
  g_tim->CR1 &= ~TIM_CR1_CEN;
  g_tim->SR = TIM_SR_UIF;
  g_tim->CNT = 0;
  g_tim->ARR = us;
  g_tim->EGR |= TIM_EGR_UG;
  g_tim->CR1 |= TIM_CR1_CEN;
  g_timeout = timeout;
}

void nrf24_hw_cancelTimer(nrf24_t* nrf24) {
  g_tim->CR1 |= TIM_CR1_CEN;
}
/* (1) Master selection, BR: Fpclk/256 (due to C27 on the board, SPI_CLK is
 set to the minimum) CPOL and CPHA at zero (rising first edge) */
/* (2) Slave select output enabled, RXNE IT, 8-bit Rx fifo */
/* (3) Enable g_spi */
void nrf24_hw_init(nrf24_t* nrf24) {
  g_nrf24 = nrf24;
  g_spi->CR1 = SPI_CR1_MSTR | SPI_CR1_BR_1; /* (1) */  // 48/8 = 6M;
  g_spi->CR2 = SPI_CR2_SSOE | SPI_CR2_RXNEIE | SPI_CR2_FRXTH
   | SPI_CR2_DS_2 | SPI_CR2_DS_1 | SPI_CR2_DS_0; /* (2) */
  g_spi->CR1 |= SPI_CR1_SPE;

  g_tim->CR1 = TIM_CR1_OPM | TIM_CR1_URS;
  g_tim->CR2 = TIM_CR2_UIE;
  g_tim->PSC = 47;

}
