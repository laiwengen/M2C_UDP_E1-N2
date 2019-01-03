/**
  ******************************************************************************
  * @file    stm32f0xx_it.c
  * @date    10/09/2015 13:58:35
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  *
  * COPYRIGHT(c) 2015 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"
#include "stm32f0xx.h"
#include "stm32f0xx_it.h"
/* USER CODE BEGIN 0 */
#include "frameworks/esp8266.h"
/* USER CODE END 0 */
/* External variables --------------------------------------------------------*/

extern DMA_HandleTypeDef hdma_adc;
extern TIM_HandleTypeDef htim1;

/******************************************************************************/
/*            Cortex-M0 Processor Interruption and Exception Handlers         */ 
/******************************************************************************/

/**
* @brief This function handles DMA1 Channel 1 Interrupt.
*/
void DMA1_Channel1_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel1_IRQn 0 */

  /* USER CODE END DMA1_Channel1_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_adc);
  /* USER CODE BEGIN DMA1_Channel1_IRQn 1 */

  /* USER CODE END DMA1_Channel1_IRQn 1 */
}
//void DMA1_Channel2_3_IRQHandler(void) {
//	uint32_t isr = DMA1->ISR;
//	if (isr & (DMA_FLAG_TC2 | DMA_FLAG_TE2)) {
//		if (isr & DMA_FLAG_TE2) { 
//			DMA1->IFCR = DMA_FLAG_GL2;
//		} else {
//			DMA1->IFCR = DMA_FLAG_TC2;
//		}
//  	DMA1_Channel2->CCR &= ~(DMA_IT_TC | DMA_IT_TE);
////		g_transmiting = 0;
////		reloadOutDma(g_sim800_main->outStream->circularBuffer);
//		
//		//if (g_isIniting == 0xff)
//		{
//			#if 0//USE_GPRS
//			flushSim800TransmitBuffer();
//			#elif 1//USE_WIFI
//			flushEsp8266TransmitBuffer();
//			#endif
//		}
//		//else
//		{
//			//flushConsoleTransmitBuffer();
//		}
//		// t tc / e?
//	}
//	if (isr & DMA_FLAG_TC3) {
//		DMA1->IFCR = DMA_FLAG_TC3;
//		// r tc
//	}
//	if (isr & DMA_FLAG_TE3) {
//		DMA1->IFCR = DMA_FLAG_GL3;
//	}
//}

/**
* @brief This function handles TIM1 Break, Update, Trigger and Commutation Interrupts.
*/
void TIM1_BRK_UP_TRG_COM_IRQHandler(void)
{
  /* USER CODE BEGIN TIM1_BRK_UP_TRG_COM_IRQn 0 */

  /* USER CODE END TIM1_BRK_UP_TRG_COM_IRQn 0 */
  HAL_TIM_IRQHandler(&htim1);
  /* USER CODE BEGIN TIM1_BRK_UP_TRG_COM_IRQn 1 */

  /* USER CODE END TIM1_BRK_UP_TRG_COM_IRQn 1 */
}

/**
* @brief This function handles System tick timer.
*/
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  HAL_SYSTICK_IRQHandler();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
