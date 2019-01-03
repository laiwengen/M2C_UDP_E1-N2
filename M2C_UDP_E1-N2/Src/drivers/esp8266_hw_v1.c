
#include "stm32f0xx_hal.h"
#include "frameworks/esp8266.h"


//DMA_Channel_TypeDef* g_esp8266_hw_hdma = DMA1_Channel5;
//USART_TypeDef* g_esp8266_hw_huart = USART3;

////pull down reset pin of the WIFI chip, then pull up.
//static void hw_reset(void)
//{
//	//HARDWARE set the gpio below as your REST pin of WIFI chip.(would be gpio16 instead, see the sch of WIFI moudle)
//	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_0,GPIO_PIN_RESET);
//	hw_delay(10);
//	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_0,GPIO_PIN_SET);
//}

//static void hw_init(void)
//{
//	//PLATFORM
//	//Setp 0: power on
//	HAL_GPIO_WritePin(GPIOF,GPIO_PIN_7,GPIO_PIN_SET);
//	//HARDWARE set the gpio below as your REST pin of WIFI chip.
//	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_0,GPIO_PIN_RESET);
//	//HARDWARE set the gpio below as your power MOSFET control pole if you have one. otherwise ignore.
//  HAL_GPIO_WritePin(GPIOF,GPIO_PIN_7,GPIO_PIN_SET);
////  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_SET);

//	//////*over write  IGNORED *//////
//	//Step 1: init the buffer to UINT16_MAX, so that we would know the buffer has been wright.
//	//memset((void*)g_esp8266_buffer,UINT16_MAX,sizeof(g_esp8266_buffer));
//	g_esp8266_dmaRemain = &(g_esp8266_huart->hdmarx->Instance->CNDTR);//CNDTR�Ĵ����м�Ϊ���յ������ݵ�����
//																																		//��g_esp8266_dmaRemainΪһ��ָ�룬ָ�����յ����ݵ�����
//	//Step 2: Start DMA. In circle mode, dma would NEVER STOPPED.			//g_esp8266_dmaRemainΪָ���յ����ݵ�ָ��
//	//start an automatic thread to collect the byte comming from RX pin.

//	//////*over write  IGNORED *//////
//	//the element of the buffer is 16bit, and has been set by 0xffff.
//	//if new byte comes, the element should be 0x00??, otherwise, it should be 0xff??. easy to find where to read.
//	//as soon as the byte has be get by other function, please remember to set it by 0xffff. (this has already been done by the function below.)
//	HAL_UART_Receive_DMA(g_esp8266_huart, (uint8_t *)g_esp8266_buffer, ESP8266_BUFFER_SIZE);
//}

//static void hw_deinit(void)
//{
//  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_0,GPIO_PIN_RESET);
//	HAL_GPIO_WritePin(GPIOF,GPIO_PIN_7,GPIO_PIN_SET);
//}
