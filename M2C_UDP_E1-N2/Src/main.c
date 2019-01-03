/**
  ******************************************************************************
  * File Name          : main.c
  * Date               : 10/09/2015 13:58:37
  * Description        : Main program body
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

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc;
DMA_HandleTypeDef hdma_adc;

I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;

//IWDG_HandleTypeDef hiwdg;

//SPI_HandleTypeDef hspi2;
DMA_HandleTypeDef hdma_spi2_rx;

TIM_HandleTypeDef htim1;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart2_rx;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2C2_Init(void);
//static void MX_IWDG_Init(void);
//static void MX_SPI2_Init(void);
static void MX_TIM1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);

/* USER CODE BEGIN PFP */
#define Select_Flash()       HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_RESET)//GPIO_ResetBits(GPIOC, GPIO_Pin_8)
#define NotSelect_Flash()    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_SET)//GPIO_SetBits(GPIOC, GPIO_Pin_8)
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
#include "math.h"
#include "stdio.h"
#include "stdlib.h"

#include "string.h"

#include "frameworks/thread.h"
#include "frameworks/schedule.h"
#include "frameworks/list.h"
#include "frameworks/FPE.h"

#include "frameworks/Button.h"
#include "frameworks/screen.h"
#include "frameworks/time.h"
#include "frameworks/md5.h"
#include "frameworks/number.h"
//#include "Console.h"
#include "LCD.h"
//#include "CJSON.h"
#include "frameworks/Tick.h"

#include "frameworks/ESP8266.h"
#include "frameworks/wireless.h"
#include "frameworks/strings.h"
#include "ADXL345.h"

#include "mainConfig.h"
#include "md5.h"
#include "qrcode.h"
#include "cQrCode.h"

//#if !AIRJ_LCD_DISPLAY
//uint16_t xOffset,yOffset;
//#endif

	
	static void esp8266CheckForCrash(void** params);
__IO float PM_modifier = 1.2;
__IO float HCHO_modifier = 1.0;

//#define COMPRESS_BMP 0
//uint32_t g_main_lastWifiTick = RESET_WHILE_NO_DATEGET;
//uint32_t g_main_pairTick = 0;
//__IO uint8_t g_main_pairToDo = 1;

//__IO uint8_t g_main_ssidToDo = 1;
//__IO uint8_t g_main_neverUsed;
//uint32_t g_main_lastResponseTick = (UINT32_MAX - 5000);
//__IO uint8_t g_main_power = 1;

//extern uint16_t xOffset,yOffset;
//int8_t g_main_10s_countDown ;
//uint8_t g_main_is_warmUp ; 
//__IO uint32_t g_main_activeMs = SYSTEM_ATUOPOFF_MS;
//uint32_t g_battery_notify_tick = 0;
enum BUTTON_ID
{
	BUTTON_DOME = 1,
//	BUTTON_TOUCH = 2,
	BUTTON_NUMBER,
};

// defines
enum main_thread_id_t {
	MAIN_THREAD_ID_eventWatcher,
	MAIN_THREAD_ID_serverWatcher,
	MAIN_THREAD_ID_schedule,
	MAIN_THREAD_ID_button,
	MAIN_THREAD_ID_batteryFetcher,
//	MAIN_THREAD_ID_sim800Fetcher,
	MAIN_THREAD_ID_dustFetcher,
	MAIN_THREAD_ID_hts221Fetcher,
//	MAIN_THREAD_ID_t6700Fetcher,
	MAIN_THREAD_ID_lcdUpdate,
	MAIN_THREAD_ID_SIZE,
};
enum DATATYPE_ID
{
	GAS_CH2O = 0,
	GAS_TVOC,
//	GAS_PM1,
	TEMPRATURE,
	HUMIDITY,	
	MAX_GAS_TYPES,
};
enum sensor_value_t {
	SENSOR_VALUE_co2,
	SENSOR_VALUE_ch2o,
	SENSOR_VALUE_ch2oWarming,
	SENSOR_VALUE_pm2d5,
	SENSOR_VALUE_pm1d0,
	SENSOR_VALUE_pm10,
//	SENSOR_VALUE_pc0d3,
//	SENSOR_VALUE_pc1d0,
//	SENSOR_VALUE_pc2d5,
//	SENSOR_VALUE_pc5d0,
//	SENSOR_VALUE_pc10,
//	SENSOR_VALUE_pal,
	SENSOR_VALUE_temperature,
	SENSOR_VALUE_humidity,
//	SENSOR_VALUE_rssi,
//	SENSOR_VALUE_battery,
	SENSOR_VALUE_SIZE,
};


// inline functions

enum system_data_t {
	SYSTEM_DATA_page,

//	SYSTEM_DATA_menuCursorIndex,
//	SYSTEM_DATA_selectCursorIndex,
//	SYSTEM_DATA_selectSelectedIndex,
//	SYSTEM_DATA_selectSize,
	SYSTEM_DATA_pageBeforeSettings,
	SYSTEM_DATA_pageBeforeScreenOff,
	SYSTEM_DATA_lastInteractTime,
	SYSTEM_DATA_lastInteractTime1,
	SYSTEM_DATA_lastInteractTime2,
	SYSTEM_DATA_lastInteractTime3,
	SYSTEM_DATA_animationStartTime,
	SYSTEM_DATA_animationStartTime1,
	SYSTEM_DATA_animationStartTime2,
	SYSTEM_DATA_animationStartTime3,
	SYSTEM_DATA_holding,
//	SYSTEM_DATA_historyTimeType,
//	SYSTEM_DATA_historyDataType,
	SYSTEM_DATA_SIZE,
};

enum system_menuPages_t {
	SYSTEM_MENU_PAGE_screenOffTime,
	SYSTEM_MENU_PAGE_powerOffTime,
//	SYSTEM_MENU_PAGE_holdTime,
//	SYSTEM_MENU_PAGE_userMulti,
	SYSTEM_MENU_PAGE_theme,
	SYSTEM_MENU_PAGE_language,
	SYSTEM_MENU_PAGE_qrcode,
	SYSTEM_MENU_PAGE_about,
	SYSTEM_MENU_PAGE_SIZE,
};

#define SYSTEM_MENU_PAGE_0 0

#define SYSTEM_MENU_PAGE_SELECTABLE_SIZE (SYSTEM_MENU_PAGE_language + 1)

enum server_timestamp_t {
	SERVER_TIMESTAMP_time,
	SERVER_TIMESTAMP_SIZE,
};
//inline function
//#define server_fetched(k) g_server_fetched_timestamps[SERVER_TIMESTAMP_##k]
//#define server_updated(k) g_server_updated_timestamps[SERVER_TIMESTAMP_##k]

//#define system_set(k,v) g_main_system_data[SYSTEM_DATA_##k] = (v)
//#define system_set32(k,v) buffer_set(g_main_system_data,SYSTEM_DATA_##k,(v),4)
//#define system_get(k) (g_main_system_data[SYSTEM_DATA_##k])
//#define system_get32(k) (buffer_get(g_main_system_data,SYSTEM_DATA_##k,4))
//#define settings_get(k) (g_settingss_value[SYSTEM_MENU_PAGE_##k])
//#define settings_set(k, v) settingsUpdated(SYSTEM_MENU_PAGE_##k, v, 0)
#define system_set(k,v) g_main_system_data[SYSTEM_DATA_##k] = (v)
#define system_set32(k,v) buffer_set(g_main_system_data,SYSTEM_DATA_##k,(v),4)
#define system_get(k) (g_main_system_data[SYSTEM_DATA_##k])
#define system_get32(k) (buffer_get(g_main_system_data,SYSTEM_DATA_##k,4))

#define sensor_set(k,v) (g_main_sensor_value[SENSOR_VALUE_##k] = (v))
#define sensor_get(k) g_main_sensor_value[SENSOR_VALUE_##k]

#define server_fetched(k) g_main_server_fetched_timestamps[SERVER_TIMESTAMP_##k]
#define server_updated(k) g_main_server_updated_timestamps[SERVER_TIMESTAMP_##k]

#define settings_get(k) (g_settingss_value[SYSTEM_MENU_PAGE_##k])
#define settings_set(k, v) settingsUpdated(SYSTEM_MENU_PAGE_##k, v, 0)

#define enabled(k) (g_enabled & (1<<SYSTEM_ENABLE_##k))


uint8_t g_main_uiChanged = 0;
uint8_t g_main_powerChanged = 0;
//static uint8_t g_enabled;


// global variables
uint32_t g_main_threadIds[MAIN_THREAD_ID_SIZE] = {0};	
//console_t* g_main_console = NULL;
uint8_t g_main_system_data[SYSTEM_DATA_SIZE] = {0};
static uint8_t g_settingss_value[SYSTEM_MENU_PAGE_SELECTABLE_SIZE] = {0};
static uint32_t g_settingss_time[SYSTEM_MENU_PAGE_SELECTABLE_SIZE] = {0};
uint32_t g_main_sensor_value[SENSOR_VALUE_SIZE] = {0};
uint32_t g_main_server_fetched_timestamps[SERVER_TIMESTAMP_SIZE] = {0};
uint32_t g_main_server_updated_timestamps[SERVER_TIMESTAMP_SIZE] = {0};

//static uint8_t g_settingss_value[SYSTEM_SETTINGS_SIZE] = {0};
//static uint32_t g_settingss_time[SYSTEM_SETTINGS_SIZE] = {0};



	#define SCHEDULE_ID_ESP8266_INIT 0xeeff0000
	static void esp8266Init(void** params);

	#define MAIN_FPE_BASE_settings 0xe3e69500
	#define MAIN_FPE_BASE_history 0xac6d8900
	#define MAIN_FPE_BASE_serverCache 0xd5021900
	#define MAIN_FPE_BASE_imei 0x096eb800
	#define MAIN_FPE_BASE_wireless 0x75820907
	#define MAIN_FPE_BASE_calibration 0xfe859836
	#define TEST_SERVER 0
	#if WIRELESS_BIN
		#if TEST_SERVER
			static const char* const g_appurl = "http://web.test.hw99lt.com/newApp/";
			static char const* const g_pushDefaultAddress = "123.57.138.74";
			static uint16_t const g_pushDefaultPort = 59733;
		#else
			static const char* const g_appurl = "http://app.hwlantian.com";
			static char const* const g_pushDefaultAddress = "bin.hwlantian.com";
			static uint16_t const g_pushDefaultPort = 59733;
		#endif
	#else
		#if TEST_SERVER
			//udp 2.1 - 112.125.123.77:59732
		static const char* const g_appurl = "http://web.test.hw99lt.com/newApp/";
		static char const* const g_pushDefaultAddress = "112.125.123.77";
		static uint16_t const g_pushDefaultPort = 59732;				
		#else
		static const char* const g_appurl = "http://app.hwlantian.com";
		static char const* const g_pushDefaultAddress = "udp.hwlantian.com";
		static uint16_t const g_pushDefaultPort = 59732;	
		#endif
	#endif
	static const char* g_devicePrefix = "E3-";
	static const char* const g_deviceHardwareVersion = "E3 1.0.0 alpha";
static const char* const g_deviceFirmwareVersion = "E3 1.0.1 alpha";
#if WIRELESS_BIN
static const char* const g_deviceProtocol = "udp 2.1 bin";
#else
static const char* const g_deviceProtocol = "udp 2.1 json";
#endif
//	static uint8_t const g_xorKeys[8] = {1,8,6,1,2,5,2,1};
static uint8_t const* const g_uid = (uint8_t*) UID_BASE;
	

//uint8_t g_main_consoleBuffer[0x100];
//uint8_t g_main_dustBuffer1[800];
//uint8_t g_main_dustBuffer2[800];



uint16_t g_main_rotation = 0;
int16_t g_main_temp = 0, g_main_humidity = 0;
int32_t g_main_ch2o = 0;//, g_main_ch2oL = 0; g_main_ch2oH = 0;
int32_t g_main_tvoc = 0;//, g_main_ch2oL = 0; g_main_ch2oH = 0;
int32_t g_main_ch2o_2 = 0;//, g_main_ch2oL = 0; g_main_ch2oH = 0;

//uint32_t g_
int16_t g_main_battery = 100;


//__IO uint8_t RUN_MODE = 'N';
uint8_t g_main_is_charging = 0;
uint8_t g_usbUartDetected = 0;
uint8_t g_main_isShaked = 0;
uint16_t g_main_tempFix = 0;
enum wifi_status
{
	WIFI_INIT = 0,
	WIFI_SMARTING,
	WIFI_REMOTE_ERROR,
	WIFI_CONNECTED,
	WIFI_RETRYING,
	WIFI_NO_AP,
	WIFI_SERVER_ACK
};
__IO uint8_t g_main_wifi_status = WIFI_NO_AP;

//uint32_t g_main_wifi_interval = 5000;
//uint32_t g_main_wifi_getDelay = 1500;
//uint32_t g_main_wifi_lastConncetTick = 0;

//#define WIFI_RESMARTTICK 30000
//#define WIFI_MAXSMARTTICK 90000

//#define UART2_BUFFER_SIZE 64
//#define UART2_BUFFER_MASK (UART2_BUFFER_SIZE - 1)
//#define getSetting(name) (name?name:name##Default)
//uint8_t g_uart2Buffer[UART2_BUFFER_SIZE];
uint8_t g_usb_configBuffer[8] = {0};
_Bool g_isIniting = 0;

static void powerOff(void);
/******** notification functions *********/
#if 1
void main_lcdEnable(uint8_t enable);
void main_uiInit(void** params);
static void interacted(void) {
  	system_set32(lastInteractTime, tick_ms());
  }

  static void powerChanged(void) {
		g_main_powerChanged = 1;
	}

  static void uiChanged(void) {
  	// uiHistoryChanged();
//  	uiSettingsChanged();
//  	lcd_set(holding, system_get(holding));
  	lcd_set32(animationStartTime, system_get32(animationStartTime));
  	g_main_uiChanged = 1;
  }
	
	static const	uint8_t g_xorKeys[8] = {1,8,6,1,2,5,2,1};

	static int16_t getDeviceQrBytes(char* buffer) {
		// if (g_main_sim800Imei[0] == 0xff) {
		// 	return 0;
		// }
		int16_t index = 0;
		// memcpy(buffer + index, g_devicePrefix, 3);
		// index += 3;
		fortas(uint8_t, g_uid, 12) {
			index += number_toHexString(v, buffer + index, 2, 0);
		}
		buffer[index] = 0;
		return index;
	}
	static int16_t getDeviceId(char* buffer) {
		// if (g_main_sim800Imei[0] == 0xff) {
		// 	return 0;
		// }
		uint8_t source[12];
		fors(12) {
			source[i] = g_uid[i] ^ g_xorKeys[i & 0x7];
		}
		uint8_t md5[16];
		if (md5_convert(source, 12, md5)) {
			int16_t index = 0;
			memcpy(buffer + index, g_devicePrefix, 3);
			index += 3;
			forta(uint8_t, md5) {
				index += number_toHexString(v, buffer+index, 2, 0);
			}
			buffer[index++] = '\0';
			return index;
		}
		return 0;
	}
	
	static void generateQRBuffer(void) {
		if (lcd_get32(settingsQrBuffer)) {
			return;
		}
		char qrBytes[40];
		if (getDeviceQrBytes(qrBytes)) {
			char const* toConcat[10] = {g_appurl,"?deviceId=",g_devicePrefix,qrBytes};
			int8_t size = 4;
			char* hardwareUrlencode = string_urlencode(g_deviceHardwareVersion);
			if (hardwareUrlencode) {
				toConcat[size++] = "&hardware=";
				toConcat[size++] = hardwareUrlencode;
			}
			char* firmwareUrlencode = string_urlencode(g_deviceFirmwareVersion);
			if (firmwareUrlencode) {
				toConcat[size++] = "&firmware=";
				toConcat[size++] = firmwareUrlencode;
			}
			char* str = string_concat(toConcat, size);
			free(hardwareUrlencode);
			free(firmwareUrlencode);
			// char* strt = "abcd";
			if (str) {
				uint8_t* qrBuffer = qrcode_convert((uint8_t*)str, strlen(str));
				if (qrBuffer) {
					lcd_set32(settingsQrBuffer, (uint32_t)qrBuffer);
				}
				free(str);
			}
		}
	}
	
	static void pageToShut(void) {
		//if (system_get(page) > LCD_PAGE_animate) 
			if (1){
			//system_set32(animationStartTime, tick_ms());
			system_set(page,LCD_PAGE_shut);
			powerChanged();
			uiChanged();
		}
	}
//	static void pageToStart(void) {
//		system_set32(animationStartTime, tick_ms());
//		system_set(page,LCD_PAGE_start);
//		powerChanged();
//		uiChanged();
//	}	
			
		static void pageToStart(void) {

		system_set32(animationStartTime, tick_ms());
		system_set(page,LCD_PAGE_start);
		powerChanged();
		uiChanged();
	}	
#endif
	
	
/*********temperary history***************/


uint8_t isButtonPressed(uint32_t id);


#define MAINTHREAD_ID 0xeeee0002
#define ESP8266_RUN_ID 0xeeee0003
void onPowerButtonPressed(void** params);
void onModeButtonPressed(void** params);
void onMode2ButtonPressed(void** params);

void co2_tick(void** params);
void ch2o_update(void** params);
void dust_update(void** params);
void th_update(void** params);
void battery_update(void** params);
void dust_upadte(void** params);
void adxl345_update(void** params);
void touchButton(void** params);
void wifi_update(void** params);
void wifi_deal(void** params);
void updateWifiStatus(void** params);
void UART_config_B2(char *stringToSend);

void mainThread(void** params);
void pm2d5_init(void);
void hts221_init(void);
//void setLevelColor(uint8_t dataType,uint32_t data,uint16_t *color,uint8_t *level);
void pairStart(void** params);
void EnterBootLoader(void** params);
void enterSleep(void);
void output_fix(void** params);
/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2C2_Init(void);
//static void MX_IWDG_Init(void);
static void MX_TIM1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void UARTswitchChannel(uint8_t channel);
static void sensorPowerOn(void** params)
{
	#if NEW_M2_PCB
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_2,GPIO_PIN_SET);//B2 power on
	#else	
	HAL_GPIO_WritePin(GPIOF,GPIO_PIN_5,GPIO_PIN_SET);//B2 power on
	#endif
//	HAL_GPIO_WritePin(GPIOF,GPIO_PIN_7,GPIO_PIN_SET); // alx345 power on
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_SET); // co2 power on
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_SET); // ch2o power on
//	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_SET); // th power on
//	adxl345_init(&hi2c1);

}
static void sensorPowerOff(void** params)
{
	#if M2_NEW_PCB
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_2,GPIO_PIN_RESET);//B2 power on
	#else	
	HAL_GPIO_WritePin(GPIOF,GPIO_PIN_5,GPIO_PIN_RESET);//B2 power on
	#endif
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_RESET); // co2 power off
	//	HAL_GPIO_WritePin(GPIOF,GPIO_PIN_7,GPIO_PIN_RESET); // alx345 power off
//	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_RESET); // ch2o power off
//	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_RESET); // th power off

}
#define OUT_FIX_ID 0xeeee0001

static void loadFlashData(void)
{
	#if UART_FIX_HCHO
	uint32_t readoutRatio = fpe_readOr(FPE_HCHO_RATIO_ADDRESS, 100);	

	HCHO_modifier = readoutRatio/100.0  ;

	char strToSend2[8] = {0}; 
	strToSend2[6] = '\r';
	strToSend2[7] = '\n';
	number_toDecString(HCHO_modifier*100,0,strToSend2,6);
//	sprintf(strToSend2,"¼×£º%4d\r\n",(uint32_t)(HCHO_modifier*100));
	HAL_UART_Transmit(&huart1,(uint8_t *)strToSend2,8,10);
	#endif
}
static void UARTswitchChannel(uint8_t channel)
{
 if (channel)//esp8266
 {
 	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_SET); // sel UART0 cs
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_6,GPIO_PIN_RESET); // sel UART0 cs
 }
 else
 {
  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_RESET); // sel UART0 cs
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_6,GPIO_PIN_RESET); // sel UART0 cs
 
 }
 __HAL_UART_CLEAR_IT(&huart1, UART_CLEAR_OREF);
}
static void hw_init(void)
{
	
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC_Init();
  MX_I2C1_Init();
  MX_I2C2_Init();
//  MX_IWDG_Init();
//  MX_SPI2_Init();
  MX_TIM1_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
//	GPIOB->BSRRH = 0x0060;//sel cs
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_SET); // dc-dc on
	HAL_Delay(100);
#if NEW_M2_PCB
//	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_SET); // alx345 power on
#else
//	HAL_GPIO_WritePin(GPIOF,GPIO_PIN_7,GPIO_PIN_SET); // alx345 power on
#endif	
	HAL_Delay(50);
//	adxl345_init(&hi2c1);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_SET); // co2 power on
	HAL_Delay(50);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_SET); // ch2o power on
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_SET); // th power on
	
//	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_RESET); // battery charge enable

	
//	while (endTime>tick_ms())
//	{
//	if  (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_4))
//	{
//	buttonCount++;
//	}
//	HAL_Delay(1);
//	}
//	if (buttonCount>450)
//	{
//	lcd_init();
//	}
//	else
//		{
//		g_main_force_sleep = 1;
//		}
	adxl345_update(0);
//#if AIRJ_LCD_DISPLAY
//	lcd_init();	

//	lcd_erase(320,240);

//	//g_main_rotation = 0;
//	lcd_setRotation(g_main_rotation);
//	lcd_setOrigin(xOffset,yOffset);	
//#endif	

	g_main_is_charging = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_3); 

	
	HAL_Delay(50);
	HAL_ADCEx_Calibration_Start(&hadc);
	
	
//	dust_init(&hadc, g_main_dustBuffer1,g_main_dustBuffer2,sizeof(g_main_dustBuffer1));
}




static void adcResolutionBits(ADC_HandleTypeDef* h, uint32_t resolution)
{
	uint32_t tmp = h->Instance->CFGR1;
	tmp &=~ADC_CFGR1_RES;
	tmp |= resolution;
	h->Instance->CFGR1 = tmp;
	
	if ( h->DMA_Handle)
	{
		HAL_DMA_Abort(h->DMA_Handle);
		tmp = h->DMA_Handle->Instance->CCR;
		tmp &=~ (DMA_CCR_MSIZE|DMA_CCR_PSIZE);
		if (resolution == ADC_RESOLUTION6b || resolution == ADC_RESOLUTION8b)
		{
			tmp |= DMA_PDATAALIGN_BYTE|DMA_MDATAALIGN_BYTE;
		}
		else
		{
			tmp |= DMA_PDATAALIGN_HALFWORD|DMA_MDATAALIGN_HALFWORD;
		}
		h->DMA_Handle->Instance->CCR = tmp;
	}
}

uint16_t g_main_touchButton = 0;
#define ADC_BUTTON_CHANNEL 7


uint32_t g_main_co2 = 0;
void co2_tick(void** params)
{
	const uint16_t CO2T = 10;
	static uint16_t high,all;
	if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0) == GPIO_PIN_SET)
	{
		high ++;
	}
	all ++;
	if (all == 1004*CO2T)
	{
		if (high<2*CO2T)
		{
			g_main_co2 = 0;
			sensor_set(co2,g_main_co2);
			lcd_set16(valueCo2,g_main_co2);
		}
		else
		{
			g_main_co2 = (high-2*CO2T)*UINT32_C(2000)/(all - 4*CO2T);
			g_main_co2 = 1.96*min(4000,g_main_co2);	
			sensor_set(co2,g_main_co2);
			lcd_set16(valueCo2,g_main_co2);
		}
		high = 0;
		all = 0;
	
	}
	

}


static void stable(int32_t value, int32_t* average, int32_t* variance)
{
	*average += (value-*average)*0.2 + 0.5;
	int32_t v;
	if (value-*average < INT16_MAX)
	{
		v = (value-*average)*(value-*average);
	}
	else
	{
		v = INT32_MAX;
	}
	*variance += (v-*variance)*0.5 + 0.5;
}
__IO uint8_t g_adcConvertDone = 0;




#define ADC_CH2O_CHANNEL 5
#if 0
static uint32_t ch2o2Voc(int32_t ch2oValue)
{
	
	int32_t	base = 0;
	int32_t voc = 0;
	int32_t data = ch2oValue;
	const static int32_t dx[] = {40,	35,	125,	100,1700};
	const static int32_t dy[] = {210,355,235, 1700,4000};
	uint16_t i=0;
//	volatile uint8_t count = (sizeof(dx)/sizeof(dx[0]))-1;
	
	for(;i<(sizeof(dx)/sizeof(dx[0]))-1;i++)
	{
		if (data<dx[i])
		{
			break;
		}
		data -= dx[i];
		base += dy[i];
	}
//	voc = 3[dx];
	voc = base+ (data*dy[i])/dx[i];
//	voc = voc * 22400 / 92140;
//	voc = min(5000,voc);
	return voc;
}
#endif

	
	#define MAIN_CH2O_WARM_TICK 30000
	#define MAIN_CH2O_WARMING_SCHEDULE_ID 0xc3ca7291
static int8_t g_warmingTime = 16;
	static void main_ch2oWarming(void** params) {
		int8_t second = g_warmingTime;
		if (g_warmingTime > 0) {
			g_warmingTime --;
		}
		if (second <= 0) {
			lcd_set(ch2oPreparing, 0);
			schedule_cancel(MAIN_CH2O_WARMING_SCHEDULE_ID);
			return;
		}
		lcd_set(ch2oPreTime, second - 1);
		sensor_set(ch2oWarming, second - 1);
	}
	static uint32_t g_disableTick = 0;
	static void main_ch2oStartWarm(void) {
		schedule_repeat(MAIN_CH2O_WARMING_SCHEDULE_ID,main_ch2oWarming,1000,0);
		lcd_set(ch2oPreparing, 1);
		uint32_t time;
		uint32_t ticks = tick_ms()-g_disableTick;
		if (ticks > 16000) {
			time = 16;
		} else {
			time = ticks/1000 + g_warmingTime;
			time = min(time, 16);
		}
		g_warmingTime = time;
		schedule_reset(MAIN_CH2O_WARMING_SCHEDULE_ID,0);
	}
void ch2o_update(void** params)
{
		//debug
//	static uint16_t CH2O_GROUND = 405;

	
	uint8_t error = 0;
	uint16_t buffer[16] = {0};
	g_adcConvertDone = 0;
	HAL_ADC_PollForConversion(&hadc,10);
	if (!(hadc.Instance->CHSELR & (1<<ADC_CH2O_CHANNEL)))
	{
		HAL_ADC_Stop(&hadc);
		hadc.Instance->CHSELR = 1<<ADC_CH2O_CHANNEL;
		adcResolutionBits(&hadc,ADC_RESOLUTION12b);
	}
//	uint16_t tbuffer[8] = {0};
  __HAL_DMA_CLEAR_FLAG(hadc.DMA_Handle, __HAL_DMA_GET_TC_FLAG_INDEX(hadc.DMA_Handle));
	HAL_ADC_Start_DMA(&hadc, (uint32_t*)buffer,16);
//	HAL_ADC_PollForConversion(&hadc,1000);
	uint32_t startTick = HAL_GetTick();
	while(g_adcConvertDone == 0)
	{
		if (HAL_GetTick() - (startTick+2) < (UINT32_MAX>>1))
		{
			error = 1;
			break;
		}
	}
	if (error)
	{
		return;
	}
	uint32_t sum = 0;
	uint16_t cnt = 0;
//	static uint16_t newGround =405;
//	static uint16_t maxVoltage =1990;
	for (uint16_t i =0; i<sizeof(buffer)/sizeof(uint16_t);i++)
	{
		sum += buffer[i];
		cnt ++;
		
	}
	
	__IO double mv = UINT32_C(3300)*sum/cnt/(UINT32_C(1)<<12);

	if (g_main_is_charging)
	{
		mv -= 8;
	}
	mv = min(mv,2000);
	mv = max(mv,400);
	

		
	g_main_ch2o = (mv - 400)*(HCHO_MAX*HCHO_FIX_INDEX)/(2000-400);

		int32_t hchoValueAfterFCorrect = 0;
		static int32_t last_hchoValue = 50;
		int32_t dealta = (g_main_ch2o)-last_hchoValue;
		hchoValueAfterFCorrect = dealta*0.3 + last_hchoValue;
		last_hchoValue = hchoValueAfterFCorrect;
		g_main_ch2o = hchoValueAfterFCorrect;
		
	static int32_t average = 50;
	static int16_t stable = 0;
	static int16_t updown = 0;
	if (g_warmingTime > 3)
	{
		int32_t delta = (g_main_ch2o - average);
		average += delta*0.3;
		if (delta >= 2){
			updown =  min(100,updown+1);
			if (updown<0){
				updown +=4;
			}
		}
		else if (delta<=-2){
			updown =  max(-100,updown-1);
			if (updown>0){
				updown -=4;
			}
		} else {
			if (updown<0){
				updown +=3;
			}
			if (updown>0){
				updown -=3;
			}
		}
		if (abs(delta)<30 && abs(updown)<15){
			stable = min(50,stable+1);
		}else{
			stable = max(0,stable-4);
		}
		uint16_t ground = fpe_read(FPE_CH2O_GROUND);
		if (average < ground){
			ground = average;
			if (stable > 40){
				fpe_write(FPE_CH2O_GROUND,ground);
			}
		}
//		g_main_ch2o_2 = ground;
		//g_main_ch2o = updown+100;

		g_main_ch2o -= ground;

	}
	
	
//		if (g_warmingTime > 3 )
//	{
//		
//		g_main_ch2o = mv;
//	}
//	else 
	{
	g_main_ch2o *= HCHO_modifier;
	g_main_ch2o = min(3000,max(g_main_ch2o,1));
	lcd_set16(valueCh2o,g_main_ch2o);
	sensor_set(ch2o,g_main_ch2o);
	}
//	#warning TEST_CH2O
//	lcd_set16(valueCh2o,g_main_battery);
//	g_main_tvoc = ch2o2Voc(g_main_ch2o);
//	
//	g_main_tvoc = min(5000,max(g_main_tvoc,1));
}


#if 1

#define UART2_BUFFER_SIZE 64
#define UART2_BUFFER_MASK (UART2_BUFFER_SIZE - 1)
uint8_t g_uart2Buffer[UART2_BUFFER_SIZE];
void pm2d5_init(void)
{
	//HAL_GPIO_WritePin(GPIOF,GPIO_PIN_5,GPIO_PIN_SET);
	//memset(g_uart2Buffer,UINT8_MAX-1,sizeof(g_uart2Buffer));
	__IO uint8_t status = HAL_UART_Receive_DMA(&huart2, (uint8_t *)g_uart2Buffer, UART2_BUFFER_SIZE);
}

static uint32_t data2pm25(uint32_t value)
{
	#if !M1_MODE
	return min(99999,value);
	#else
	const static uint16_t dx[] = {3850,3600,8050,7450,13250,14350};
const static uint16_t dy[] = {5000,5000,10000,5200,5500,8500};
	uint8_t i;
	uint32_t pm25;

	uint32_t data = value;
	uint32_t base;
	base = 0;
	for(i=0;i<(sizeof(dx)/sizeof(uint16_t))-1;i++)
	{
		if (data<dx[i])
		{
			break;
		}
		data -= dx[i];
		base += dy[i];
	}
	pm25 = base+ (data*dy[i])/dx[i];
	//pm25=data;
	if (pm25>99999UL)
	{
		pm25 = 99999UL;
	}
	return (uint32_t)pm25;
	#endif
}

void dust_update(void** params)
{
	
	
		uint8_t tbuffer[UART2_BUFFER_SIZE] = {0};
		uint8_t start = 0;
		for(uint8_t i=0; i<UART2_BUFFER_SIZE; i++)
		{
			uint8_t c = g_uart2Buffer[i];
			 if (c == 0x5a && g_uart2Buffer[(i+1) & UART2_BUFFER_MASK] == 0xa5 && g_uart2Buffer[(i+2) & UART2_BUFFER_MASK] == 0x34)
			{
				start = i;
				
				uint16_t check = 0;
				//buffer[0] = 0;
				for( uint16_t j = 0; j<UART2_BUFFER_SIZE; j++)
				{
					tbuffer[j] = g_uart2Buffer[j+start & UART2_BUFFER_MASK];
//					if (j<=57)
//					{
//					check += tbuffer[j] ;
//					}
					
				}
				check = 0;
				for( uint16_t j = 0; j<56; j=j+4)
				{				
				//if (j<=57)
					{
					check += tbuffer[j] + tbuffer[j+1] + (tbuffer[j+2]<<8) +(tbuffer[j+3]<<16) ;
					}
				
				}
				if (((tbuffer[58]<<8)+(tbuffer[59]))==check)
				{
					uint32_t number1,number2_5,number10;
					number1 = (tbuffer[11]<<24) +(tbuffer[10]<<16) +(tbuffer[9] <<8) + (tbuffer[8]);
					number2_5 =(tbuffer[19]<<24) +(tbuffer[18]<<16) +(tbuffer[17]<<8) + (tbuffer[16]);
					number10 = (tbuffer[35]<<24) +(tbuffer[34]<<16) +(tbuffer[33]<<8) + (tbuffer[32]);
					
					{
					//	g_main_PM1Value = 	MIN(99999,number1);
						number1 = data2pm25(number1*PM_modifier);
						number2_5 = data2pm25(number2_5*PM_modifier);
						number10 = data2pm25(number10*PM_modifier);
						sensor_set(pm1d0, number1);
						sensor_set(pm2d5, number2_5);
						sensor_set(pm10, number10);
						
						lcd_set32(valuePm2d5,number2_5);
//						g_main_PM10Value =  data2pm25(number10*PM_modifier);
//						g_main_PM2_5Value = MIN(99999,number2_5*PM_modifier);
//						g_main_PM10Value 	= MIN(99999,number10*PM_modifier);
					}
				}
				
			}
			else if (c == 0xAA && g_uart2Buffer[(i+6) & UART2_BUFFER_MASK] == 0xFF)
			{
				
		//		char str[15] = "at+outtype=4\r\n";
						UART_config_B2("at+outtype=4\r\n");	
			
			}
		}
	
//UART_config_B2("at+fco\r\n");	
UART_config_B2("at+upper=0\r\n");
}


#endif


typedef struct
{
	uint8_t H0_rH;
	uint8_t H1_rH;
	int16_t H0_T0_out;
	int16_t H1_T0_out;
	
	uint16_t  T0_degC;
	uint16_t  T1_degC;
	int16_t T0_out; 
	int16_t T1_out;	
	
	int16_t T_out;
	int16_t H_out;
}hts221_t;
hts221_t hts221_var;
//#define TH_I2C_ADDRESS 0xe0
#define ST_I2C_ADDRESS 0xBE
void hts221_init(void)
{
	uint8_t setupBytes[] = {0x81};
	uint8_t sendBytes[] = {0xB0};
	uint8_t retries = 3;
	while(retries>0)
	{
		if	(HAL_I2C_Mem_Write(&hi2c2,ST_I2C_ADDRESS,0x20,I2C_MEMADD_SIZE_8BIT, setupBytes,sizeof(setupBytes),15)==HAL_OK)
		{
		if (HAL_I2C_Master_Transmit(&hi2c2,ST_I2C_ADDRESS,sendBytes,sizeof(sendBytes),30) == HAL_OK)
		{	
			
		uint8_t receiveBytes[16]= {0};
		memset(receiveBytes,0xff,16);
		//HAL_Delay(10);
		if (HAL_I2C_Master_Receive(&hi2c2,ST_I2C_ADDRESS,receiveBytes,sizeof(receiveBytes),15) == HAL_OK)
		{
			hts221_var.T0_degC = (receiveBytes[2] | ((uint16_t)receiveBytes[5] & 0x03 )<<8)>>3;
			hts221_var.T1_degC = (receiveBytes[3] | ((uint16_t)receiveBytes[5] & 0x0C )<<6)>>3;
			hts221_var.T0_out = receiveBytes[12] | ((int16_t)receiveBytes[13]<<8 );
			hts221_var.T1_out = receiveBytes[14] | ((int16_t)receiveBytes[15]<<8 );
			
			hts221_var.H0_rH = receiveBytes[0]>>1;
			hts221_var.H1_rH = receiveBytes[1]>>1;
			hts221_var.H0_T0_out = receiveBytes[6] | ((uint16_t)receiveBytes[7]<<8 );
			hts221_var.H1_T0_out = receiveBytes[10] | ((uint16_t)receiveBytes[11]<<8 );
			break;
		}
		}
		}
		retries--;
	}
}
void tempFixTick(void** params)
{
	if (g_main_tempFix<(1<<15))
	{
		g_main_tempFix += (((1<<15) - g_main_tempFix)>>10)+1;
	}
}

float getFixedTemp(float temp)
{
	if (temp<10)
	{
		return temp;
	}
	else
	{
		temp -= (temp-10)*0.15*(g_main_tempFix/(float)(1<<15));
	}
	return temp;
}
float getFixedHumi(float humi)
{
	humi += max(humi,0)*(0.5*(g_main_tempFix/(float)(1<<15)));
	return min(humi, 100);
}
#define TH_CHECK_END_ID 0xeeee0004
void th_check_end(void** params)
{
//	uint8_t receiveBytes[6]= {0};
//	if (HAL_I2C_Master_Receive(&hi2c2,TH_I2C_ADDRESS,receiveBytes,sizeof(receiveBytes),10) == HAL_OK)
//	{
//		uint16_t temperatureData = (((uint16_t)receiveBytes[0])<<8) + receiveBytes[1];
//		g_main_temperature = ((1000*UINT64_C(175)*temperatureData)>>16);
//		uint16_t humidityData = (((uint16_t)receiveBytes[3])<<8) + receiveBytes[4];
//		g_main_humidity = ((1000*UINT32_C(100)*humidityData)>>16);
//	}
//int32_t g_tempBeforeFix = INT32_MAX;
	uint8_t receiveBytes[5]= {0};
	memset(receiveBytes,0xff,5);
if (HAL_I2C_Master_Receive(&hi2c2,ST_I2C_ADDRESS,receiveBytes,sizeof(receiveBytes),15) == HAL_OK)
{
	if (receiveBytes[0]==0x03)//check data ready register
	{
		int32_t nTemperature = 0;
		hts221_var.T_out = (int16_t)(((uint16_t)receiveBytes[4] << 8) | receiveBytes[3] ); //actual temperature before calibration		
		float slope = ( hts221_var.T0_degC*1.0 - hts221_var.T1_degC)/(hts221_var.T0_out*1.0 - hts221_var.T1_out); //slope of temp calibration line
		float intercept = hts221_var.T0_degC*1.0  - slope * hts221_var.T0_out ; 							//intercept of temp calibration line
		nTemperature = (slope * hts221_var.T_out + intercept)*100;
		int32_t orgin_number = min(12000,max(-2000,nTemperature));
		int32_t delta = 0; 
		if (orgin_number>1500) 
		{
			delta = orgin_number - (1500 + (orgin_number-1500)*0.8);
			delta = min(300,delta);
		//	g_main_temp = orgin_number - delta;
	//	g_main_temp = min(8000,tempBeforeFix - max(tempBeforeFix-15,0)*0.7 ); 	
		}
		g_main_temp = orgin_number - delta;
		g_main_temp = min(12000,max(-200,g_main_temp));
		//g_main_temp = getFixedTemp(g_main_temp);
	//	g_main_temp = g_temp*100;
		//actual temperature
//		int16_t diff = 	FLASH_ReadHalfWord(TEMPERATURE_ADDRESS,g_pageFlag);
//	if (diff== -1)
//	{
//	diff = 0;
//	}

//g_temperature = g_tempBeforeFix + diff/100;
	//	g_main_temperature = 	g_tempBeforeFix;
		
		int32_t nHumidity = 0;
		hts221_var.H_out = (int16_t)(((uint16_t)receiveBytes[2] << 8) | receiveBytes[1] );//actual humidity before calibration
		slope = ( hts221_var.H0_rH*1.0 - hts221_var.H1_rH)/(hts221_var.H0_T0_out*1.0 - hts221_var.H1_T0_out);//slope of humi calibration line
		intercept = hts221_var.H0_rH*1.0  - slope * hts221_var.H0_T0_out;										//intercept of humi calibration line
		nHumidity = (slope * hts221_var.H_out + intercept)*108;
		nHumidity = min(9500,max(0,nHumidity));															//actual humidity
//		stat_handler->comfortablelevel = air_comfortable_level(max(0,g_temperature),(uint8_t)g_humidity);	
		
//		#if DEBUG_RAND
//static uint32_t testnumber = 0;
//			testnumber++;
//				if (testnumber>0x3f) testnumber = 0;
//nHumidity=nTemperature = testnumber;
//#endif
//		g_main_temp = nTemperature;
		g_main_humidity = nHumidity;
		sensor_set(temperature,g_main_temp);
		lcd_set16(valueTemperature,g_main_temp);
		sensor_set(humidity,g_main_humidity);
		lcd_set16(valueHumidity,g_main_humidity);		
//		lcd_set16(valueTemperature,-5432);

		
	//	g_main_temp_humidity = (nTemperature<<8) | nHumidity;
	}	
}
			
		//	g_main_humidity = getFixedHumi(g_main_humidity);
}

void th_update(void** params)
{
//	uint8_t sendBytes[] = {0x7c,0xa2};
//	HAL_I2C_Master_Transmit(&hi2c2,TH_I2C_ADDRESS,sendBytes,sizeof(sendBytes),10);
//	delayCall_call(th_check_end,300,1,0);
		uint8_t sendBytes2[] = {0xA7};//ST
		if (HAL_I2C_Master_Transmit(&hi2c2,ST_I2C_ADDRESS,sendBytes2,sizeof(sendBytes2),30) == HAL_OK)
		{
//			delayCall_call(th_check_end,30,1,0);	
			schedule_once(TH_CHECK_END_ID,th_check_end,30,0);
		}

}
/******** battery functions *********/
#if 1

	#define MAIN_BATTERY_SCHEDULE_ID 0x943de682
	static void main_batteryTimer(void** params) {
		
		static uint8_t blend;
		blend = (g_main_battery<20 && !g_main_is_charging)?!blend:0;
		lcd_set(batteryBlend, blend);
	  lcd_set(batteryCharging, g_main_is_charging);
		static uint8_t index = 0xff;
		uint8_t realIndex = min(g_main_battery, 100);
		if (index == 0xff) {
			index = realIndex;
			lcd_set(batteryRealIndex, realIndex);
		} else if (g_main_is_charging) {
			index +=5;
			if (index >= 105) {
				index = realIndex;
			}
		} else {
//			if (realIndex >= 100)
//			{
//				realIndex = 100;
//			}
			index = realIndex;
		}
		index = min(100,index);
	  lcd_set(batteryIndex, index);
		 // lcd_set(batteryIndex, 3);
//		if (lcd_get(batteryRealIndex) <= realIndex)
		{
			lcd_set(batteryRealIndex, realIndex);
		}
		if (index <= 5 && !g_main_is_charging){
			if (system_get(page) > LCD_PAGE_animate || (system_get(page) == LCD_PAGE_start  && tick_ms() > 600)){
				system_set32(animationStartTime, tick_ms());
				system_set(page,LCD_PAGE_low_power);
				powerChanged();
				uiChanged();
		} 
	}
	}
	#endif
#define ADC_BATTERY_CHANNEL 6
#define ADC_CURRENT_CHANNEL 9
void battery_update(void** params)
{
	uint8_t error = 0;
	uint16_t buffer[16] = {0};
	g_adcConvertDone = 0;
//	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_SET); // battery charge disable
	HAL_ADC_PollForConversion(&hadc,10);
	if (!(hadc.Instance->CHSELR & (1<<ADC_BATTERY_CHANNEL)))
	{
		HAL_ADC_Stop(&hadc);
		hadc.Instance->CHSELR = 1<<ADC_BATTERY_CHANNEL;
		adcResolutionBits(&hadc,ADC_RESOLUTION12b);
	}
	
	HAL_ADC_Start_DMA(&hadc, (uint32_t*)buffer,sizeof(buffer)/sizeof(uint16_t));
//	HAL_ADC_PollForConversion(&hadc,2);
		uint32_t startTick = HAL_GetTick();
	while(g_adcConvertDone == 0)
	{
		if (HAL_GetTick() - (startTick+5) < (UINT32_MAX>>1))
		{
			error = 1;
			break;
		}
	}
	if (error)
	{
		return;
	}
	
		uint32_t sum = 0;
	uint16_t cnt = 0;
	for (uint16_t i =1; i<sizeof(buffer)/sizeof(uint16_t);i++)
	{
		sum += buffer[i];
		cnt ++;
	}
	
	int32_t voltage = UINT32_C(6600)*sum/cnt/(UINT32_C(1)<<12) + 20;
	
	static int32_t average = 0, variance = 1000;

	if (g_main_is_charging)
	{
		voltage = 1.35*voltage - 1562;
		voltage = max(2500,voltage);		
//		fpe_write(0x0108,g_main_battery);
	}
	stable(voltage,&average,&variance);
	
	
//	if (variance>25)
//	{
//		g_main_battery = 100;
//	}
// 	else
		__IO uint16_t valueBeforeFC = 0;
		uint16_t valueAfterFC = 0;
	//	uint16_t valueAfterFC2 = 0;
	uint16_t maxVoltage=0;
	if (g_main_is_charging)
	{
		maxVoltage = 4000;
		
	}
	else
	{
		maxVoltage = 3950;

	}
	{
		if (voltage < 3300)
		{
			valueBeforeFC = 0;
		}
		else if( voltage > maxVoltage)
		{
			valueBeforeFC = 105;
		}
		else
		{
			valueBeforeFC = (voltage - 3300)*100/(maxVoltage - 3300);
		}
	}
	//valueBeforeFC = voltage;
	uint8_t USBtest = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_3);
	if ( USBtest )
	{		

		g_main_is_charging = 1;
	}
	else 
	{
		g_main_is_charging = 0;
	
	}
	static uint16_t lastValue = 0;
//	static uint16_t lastValue2 = 50;
	int16_t dealta = valueBeforeFC - lastValue;	
	if (g_main_is_charging == 0 && dealta>0)
	{
	dealta = dealta /2;
	}
	valueAfterFC = lastValue + dealta*0.4;
	lastValue = valueAfterFC;	

	if (USBtest == 0)
	{		
	//	if (valueAfterFC2<valueAfterFC)
		{
		g_main_battery = valueAfterFC;
		}
		if (g_main_battery <=3)
		{
//			#if AIRJ_LCD_DISPLAY
//			lcd_erase(320,240);
//			lcd_showlogo(rgb2c(255,255,255),1);
//			HAL_Delay(500);
//			lcd_erase(320,240);
//			HAL_Delay(500);
//			lcd_showlogo(rgb2c(255,255,255),1);
//			HAL_Delay(500);
//			lcd_erase(320,240);
//			#endif
//			g_main_status = STATUS_POWEROFF;
//			enterSleep();
		}
//		if (tick_ms() - g_main_activeMs < (UINT32_MAX>>1)) 
//		{
//			#if AIRJ_LCD_DISPLAY
//			lcd_erase(320,240);
//			#endif
//			g_main_status = STATUS_POWEROFF;
//			enterSleep();
//		}		
	}
	else
	{
//		g_main_activeMs = tick_ms() + SYSTEM_ATUOPOFF_MS;
		g_main_battery = valueAfterFC;		
	}
	
//	g_main_battery = voltage;	
	//else  if (g_main_is_charging && g_isIniting)
//	{
//		if (g_main_battery <=100)
//		{
//		g_main_status = STATUS_POWEROFF;
//		}
//	}
//#warning TEST BATTERY	
//g_main_battery = 3;
	
}

void adxl345_update(void** params)
{

	static uint16_t rotation = 0;
	static uint32_t startTick = 0;
	int16_t x,y,z;
//	uint8_t isShaked= 0;
//	if (adxl345_getShaking())
//	{
//	g_main_isShaked = 1;
//		return;
//	}
	
	if (!adxl345_getXYZ(&x,&y,&z))
	{
		startTick = tick_ms();
//		if (isShaked)
//	{
//		
////	g_main_PM2_5Value = isShaked;
//	}
		return;
	}
	//	if (g_main_status == STATUS_HISTORY)
	
//	if ( (abs(y)>0x11f) && ((x*x) + (y*y) + (z*z)> (2*0xff*0xff) ))
//	{
//		g_main_isShaked = 0xaa;
//			return;
//		}
//		if (g_main_isShaked == 0xaa &&((x*x) + (y*y) + (z*z) < (1.2*0xff*0xff) ) )//&& (abs(y)<0x7f))
//		{
//			g_main_isShaked = 1;
//			g_main_activeMs = tick_ms() + SYSTEM_ATUOPOFF_MS;
//		}
//	
//	console_printf(g_main_console,"x= %i, y=%i, z=%i \r\n",x,y,z);
	if(abs(z)>abs(x) && abs(z)>abs(y) &&g_isIniting !=1)
	{
		startTick = tick_ms();
		return; 
	}
	__IO uint16_t r;
		if (g_isIniting && (abs(x)<0x5f) && (abs(y)<0x5f) )
	{
		r = 2;			
	}
	else if(x>abs(y)*2)
	{
		r = 1;
	}
	else if(-x>abs(y)*2)
	{
		r = 3;
	}
	else if(y>abs(x)*2)
	{
		r = 0;
	}
	else if(-y>abs(x)*2)
	{
		r = 2;
	}
	else
	{
		startTick = tick_ms();
		return;
	}
	if(r != rotation)
	{
		rotation = r;
		startTick = tick_ms();
	//	return;
	}
	r = r*90+180;			
	if (r>=360)
	{
		r-= 360;
	}
	if (r!= g_main_rotation)
	{
		if (tick_ms() - (startTick + 300) <(UINT32_MAX>>1) )
		{
			g_main_rotation = r;			
			lcd_setRotation(g_main_rotation);
			lcd_set(direction,g_main_rotation/90);
			uiChanged();
			
			//console_printf(g_main_console,"rotation = %d \r\n",r*90);
		}
	}
	
	
	
}


#if 0
void updateWifiStatus(void** params)
{
	if (g_main_status == STATUS_POWEROFF)
	{
		return;
	}
	uint8_t status = esp8266_getStatus();
	switch(status)
	{
		case 1: //smart link
		{
		//	setLedList(LED_LIST_RAINBOW,sizeof(LED_LIST_RAINBOW)>>2,5000);
			g_main_wifi_status = WIFI_SMARTING;
			break;
		}
		case 3: //connceted		
		case 4: //unconncet
		{
			
//			if (g_main_lastResponseTick+5000 - tick_ms() < (UINT32_MAX>>1))
			{
//				if (g_main_requestLedList && g_main_requestLedListSize && g_main_requestLedListTime)
//				{
//					setLedList(g_main_requestLedList,g_main_requestLedListSize,g_main_requestLedListTime);
//				}
//				else
//				{
//					setLedList(LED_LIST_GREEN_BREATH,sizeof(LED_LIST_GREEN_BREATH)>>2,3000);
//				}
				g_main_wifi_status = WIFI_SERVER_ACK;
//				if ( g_main_neverUsed )
//				{
//					fpe_write(FPE_NEVERUSED_ADDRESS,0);
//					g_main_neverUsed = 0;
//				}
			}
//			else 
			{
				if (status == 3)
				{
				g_main_wifi_status = WIFI_CONNECTED;
				}
				else
				{
				g_main_wifi_status = WIFI_REMOTE_ERROR;
				}
				//setLedList(LED_LIST_YELLOW_BLINK,sizeof(LED_LIST_YELLOW_BLINK)>>2,200);
				
			}
			break;
		}
		case 5: //no wifi
		{
			g_main_wifi_status = WIFI_NO_AP;
			//setLedList(LED_LIST_BLUE_BLINK,sizeof(LED_LIST_BLUE_BLINK)>>2,200);
			break;
		}
		case 2: //no wifi
		{
			g_main_wifi_status = WIFI_REMOTE_ERROR;;
			//setLedList(LED_LIST_BLUE_BLINK,sizeof(LED_LIST_BLUE_BLINK)>>2,200);
			break;
		}
	}
}
static void updateStringSetting(char* value, char** pCurrent)
{
	if (*pCurrent)
	{
		free(*pCurrent);
	}
	if (strlen(value) == 0)
	{
		*pCurrent = NULL;
	}
	else
	{
		char* copy = malloc(strlen(value)+1);
		if (copy)
		{
			strcpy(copy,value);
			*pCurrent = copy;
		}
	}
}
#endif
#if 0
char* g_main_tokenString = NULL;
__IO uint32_t g_main_wifi_interval = 1000;
void wifi_response(void** params)
{
	if (g_main_status == STATUS_POWEROFF)
	{
		return;
	}
	char* receive = esp8266_getReceiveString();
	if (!receive)
	{
		if(tick_ms() - g_main_lastWifiTick < (UINT32_MAX>>1))
		{
			if (!esp8266_smarting())
			{
				esp8266_reset();
				g_main_lastWifiTick = tick_ms() + RESET_WHILE_NO_DATEGET;
			}
		}
	}
	else
	{
		g_main_lastWifiTick = tick_ms() + RESET_WHILE_NO_DATEGET;
	}
	if (receive)
	{
		cJSON* json = cJSON_Parse(receive);
		if(json)
		{
			g_main_lastResponseTick = tick_ms();
			cJSON* child;
			child = cJSON_GetObjectItem(json,"udpAddress");
			if (child)
			{
				updateStringSetting(child->valuestring, &g_main_udpAddress);
			}
			child = cJSON_GetObjectItem(json,"udpPort");
			if (child)
			{
				g_main_udpPort = child->valueint;
			}
			child = cJSON_GetObjectItem(json,"setUid");
			if (child)
			{
				#if !MAC2UID
				updateStringSetting(child->valuestring, &g_main_uid);
				char* uid = child->valuestring;
				fpe_writeString(FPE_UID_ADDRESS,uid);
				#endif
			}	
			if (g_main_pairToDo)
			{
				child = cJSON_GetObjectItem(json,"pairConfirmed");
				if (child)
				{
					int16_t confirmed = child->valueint;
					int16_t pairTick = tick_ms() - g_main_pairTick;
					if (abs(confirmed-pairTick)<1000)
					{
						g_main_pairToDo = 0;
					}
				}
			}
			if (g_main_ssidToDo)
			{
				child = cJSON_GetObjectItem(json,"ssidConfirmed");
				if (child)
				{
					char* confirmed = child->valuestring;
					char* ssid = esp8266_getSSID();
					if (strcmp(confirmed,ssid) == 0)
					{
						g_main_ssidToDo = 0;
					}
				}
			}
			child = cJSON_GetObjectItem(json,"slope");
			if (child)
			{
				uint32_t modifier =  child->valueint;
				if (modifier != (PM_modifier*100)) 
				{
					PM_modifier = (child->valueint)/100.0;
					fpe_write(PM_RATIO_ADDRESS,(uint32_t)(PM_modifier*100));
				}					
			}
			child = cJSON_GetObjectItem(json,"setInterval");
			uint32_t interval = 0;
			if (child)
			{
				interval = child->valueint;
			}	
			if (interval>0)
			{
				if (interval != g_main_wifi_interval)// || g_main_wifi_status != WIFI_CONNECT)
				{
					g_main_wifi_interval = interval;
					thread_remove(0,MAINTHREAD_ID);
					thread_t* t = (thread_t*) malloc(sizeof(thread_t));
					
					t->remainTimes = 0;
					t->executeTick = tick_ms()+g_main_wifi_interval;
					t->function = mainThread;
					t->intervalTick = g_main_wifi_interval;
					//t->minNeedTick = 0;
//					t->next = NULL;
					t->params = 0;
					if (thread_add(0,t,ESP8266_RUN_ID) == 0)
					{
						free(t);
					}
				//	g_main_wifi_getDelay = 5000;
				//	g_main_wifi_triggerTick = tick_ms()+g_main_wifi_interval+g_main_wifi_getDelay;
				}
			}
			cJSON* AQIcontent;
			AQIcontent = cJSON_GetObjectItem(json,"air");

			if (AQIcontent)
			{
				child = cJSON_GetObjectItem(AQIcontent,"pm25");
				if (child)
				{
//					cJSON* nexgtChild;
//					nexgtChild = cJSON_GetObjectItem(child,"pm25");				
					if (child->valuedouble)
					{
						g_main_outDoor_pm25 = (child->valuedouble)*100;
					}
				}

			}
			child = cJSON_GetObjectItem(json,"token");
			if (child)
			{
				cJSON* token = cJSON_GetObjectItem(child,"value");
				if (token)
				{
					if (g_main_tokenString)
					{
						free(g_main_tokenString);
					}
					uint8_t len = strlen(token->valuestring);
					g_main_tokenString = malloc(len + 1);
					strncpy(g_main_tokenString,token->valuestring,len);				
				
				}			
			}
			cJSON_Delete(json);
		}
		free(receive);
	}
}

void wifiSmartLinkStarted(void)
{
	g_main_ssidToDo = 1;
	pairStart(0);
}

void updateOutdoorAQI(void** params)
{
	if (g_main_status == STATUS_POWEROFF)
	{
		return;
	}
	cJSON* json = cJSON_CreateObject();
	if (json)
	{
		cJSON* token = cJSON_CreateObject();
		cJSON_AddItemToObject(json,"token",token);
		cJSON_AddStringToObject(token,"devId",getSetting(g_main_uploadId));
		uint8_t idBuffer[16];
		MD5_CTX md5;
		MD5Init(&md5);         		
		MD5Update(&md5,(unsigned char *)(g_main_tokenString),strlen(g_main_tokenString));		
		MD5Final(&md5,idBuffer);  
		
		char tokenString[32] = {0};
		if (tokenString)
		{
			for (uint8_t i = 0; i < 16;i ++)
			{
				sprintf(tokenString + strlen(tokenString),"%02X",idBuffer[i]);
			}
			cJSON_AddStringToObject(token,"value",tokenString);
			free(tokenString);
			
		}
		
		
		
		char * str = cJSON_PrintUnformatted(json);
		//char* buffer = "5f6e8f455f6f27d4";
		if (str)
		{
			char* address = getSetting(g_main_udpAddress);//?g_main_udpAddress:g_main_udpAddressDefault;
			uint16_t port = getSetting(g_main_udpPort);//g_main_udpPort!=0?g_main_udpPort:g_main_udpPortDefault;
			esp8266_udp(str,address,port,1);
			free(str);
		}
		cJSON_Delete(json);
	}


}
#endif
#if 0

void mainThread(void** params)
{
	if (g_main_status == STATUS_POWEROFF)
	{
		return;
	}
	static uint16_t count = 0;
	
	count = (count + 1) & 0x07;
	if (count == 0x07)
	{
		updateOutdoorAQI(0);
		return;
	}

	//th_update();
	cJSON* json = cJSON_CreateObject();
	if (json)
	{
		#if MAC2UID
		cJSON_AddStringToObject(json,"devId",getSetting(g_main_uploadId));
		#else
		cJSON_AddStringToObject(json,"devId",getSetting(g_main_uid));
		#endif

		cJSON_AddStringToObject(json,"ver",DEVICE_VERSION);
		cJSON_AddNumberToObject(json,"slope",(uint32_t)PM_modifier*100);
		if (g_main_power)
		{
			cJSON_AddNumberToObject(json,"pm2d5",g_main_PM2_5Value/100.0);
		//	cJSON_AddNumberToObject(json,"pm1d0",g_main_PM1Value/100.0);
			cJSON_AddNumberToObject(json,"pm10",g_main_PM10Value/100.0);
			cJSON_AddNumberToObject(json,"ch2o",g_main_ch2o);
			cJSON_AddNumberToObject(json,"temp",g_main_temp/100.0);
			cJSON_AddNumberToObject(json,"hum",g_main_humidity/100.0);
		}
		char* ssid = esp8266_getSSID();
		if (ssid)
		{
			cJSON_AddStringToObject(json,"ssid",ssid);
		}
		if (g_main_pairToDo)
		{
			uint32_t pairTick = tick_ms() - g_main_pairTick;
			if (pairTick > 120000)
			{
				g_main_pairToDo = 0;
			}
			else
			{
				cJSON_AddNumberToObject(json,"pair",pairTick);
			}
		}
		cJSON_AddNumberToObject(json,"systemTick",tick_ms());
		char * str = cJSON_PrintUnformatted(json);
		cJSON_Delete(json);
		if (ssid)
		{
			free(ssid);
		}
		if (str)
		{
			uint16_t length = strlen(str);
			char* buffer = malloc(length+2);
			if (buffer)
			{
				//sprintf(buffer,"%s\n",str);
				char* address = getSetting(g_main_udpAddress);//?g_main_udpAddress:g_main_udpAddressDefault;
				uint16_t port = getSetting(g_main_udpPort);//g_main_udpPort!=0?g_main_udpPort:g_main_udpPortDefault;
				//esp8266_udp(buffer,address,port);
				esp8266_udp(str,address,port,0);
				free(buffer);
			}
			free(str);
		}
	}
	
}
#endif
#if 0

void mainThread(void** params)
{
	if (g_main_status == STATUS_POWEROFF)
	{
		return;
	}
	static uint16_t count = 0;
	
	count = (count + 1) & 0x07;
	if (count == 0x07)
	{
		updateOutdoorAQI();
		return;
	}
//	th_update();
	cJSON* json = cJSON_CreateObject();
	if (json)
	{
		cJSON* contentsArray = cJSON_CreateArray();
		if (contentsArray)
		{
			cJSON* contents = cJSON_CreateObject();
			if (contents)
			{
				//cJSON_AddStringToObject(json,"devId",getSetting(g_main_uploadId));
				cJSON_AddStringToObject(contents,"deviceId",getSetting(g_main_uploadId));
				cJSON* data = cJSON_CreateObject();
				if (data)
				{			
					cJSON_AddNumberToObject(data,"pm2d5",g_main_PM2_5Value/100.0);

					cJSON_AddNumberToObject(data,"pm10",g_main_PM10Value/100.0);

					cJSON_AddNumberToObject(data,"ch2o",g_main_ch2o);
					cJSON_AddNumberToObject(data,"temp",g_main_temp/100.0);
					cJSON_AddNumberToObject(data,"hum",g_main_humidity/100.0);
					
					cJSON_AddItemToObject(contents, "data", data);
				}

	
					
			
		
		cJSON_AddItemToArray(contentsArray,contents);			
			}
		}
		cJSON_AddItemToObject(json,"contents",contentsArray);
		char * str = cJSON_PrintUnformatted(json);
		cJSON_Delete(json);

		if (str)
		{
			uint16_t length = strlen(str);
			char* buffer = malloc(length+2);
			if (buffer)
			{
				sprintf(buffer,"%s\n",str);
				char* address = getSetting(g_main_udpAddress);//?g_main_udpAddress:g_main_udpAddressDefault;
				uint16_t port = getSetting(g_main_udpPort);//g_main_udpPort!=0?g_main_udpPort:g_main_udpPortDefault;
				esp8266_udp(buffer,address,port,0);
				free(buffer);
			}
			free(str);
		}
	}

}
#endif
void UART_config_B2(char *stringToSend)
{	
	
	HAL_UART_Transmit(&huart2,(uint8_t *)stringToSend,strlen(stringToSend),100);
	
}
#if 1
#define SCHEDULE_OUT_FIX_ID 0xede0000 
void output_fix(void** params)
{	
		//HAL_UART_Receive(&huart1,tbuffer,8,200);
		for(uint16_t i=0; i<6; i++)
		{
			uint8_t c = g_usb_configBuffer[i];
			#if UART_FIX_HCHO
			if (c== 'L' && g_usb_configBuffer[(i+1)&0x07] == 'H')
			{
				g_usbUartDetected = 1;
				if (g_usb_configBuffer[(i+2)&0x07] == '+')
				{
					HCHO_modifier *= 1.05;				
				}
				else if (g_usb_configBuffer[(i+2)&0x07] == '-')
				{
					HCHO_modifier /= 1.05;				
				}
				HCHO_modifier = max(0.4,HCHO_modifier);

			//	uint8_t check = c + g_uart2Buffer[i+1] + runMode;
			//	if (ratio == 'N' || ratio == 'M')
				{
				fpe_write(FPE_HCHO_RATIO_ADDRESS,(uint32_t)(HCHO_modifier*100));
				}	
				char strToSend[8] = {0}; 
				strToSend[6] = '\r';
				strToSend[7] = '\n';
				number_toDecString((HCHO_modifier*100), 0 , strToSend, 6);
				//sprintf(strToSend,"%4d",(uint32_t)(HCHO_modifier*100));
				HAL_UART_Transmit(&huart1,(uint8_t *)strToSend,8,10);
				//HAL_UART_Transmit(&huart1,tbuffer[(i+2)&0x07],1,100);
				break;
			}
			#endif
		}
	memset(g_usb_configBuffer,0xff,8);
//	HAL_UART_Transmit(&huart1,"test",4,10);
}
#endif

/******** lcd functions *********/
#if 1
	static uint8_t g_lcdEnable = 1;
  static void main_lcdUpdate(void** params) {
    if (g_lcdEnable == 0) {
      return;
    }
  	if (g_main_uiChanged) {
  		g_main_uiChanged = 0;
  	}
		
		lcd_set(wifiStatus,esp8266_getStatus());
  	lcd_set(page, system_get(page));
//  	lcd_set(page, LCD_PAGE_all);
 // 	lcd_set(theme, settings_get(theme));
//  	lcd_set(language, settings_get(language));
  	lcd_update();
  }

	#define LCD_INIT_SCHEDULE_ID 0xcfd448f0

  void main_lcdEnableStep4(void** params) {
		
    //HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12,GPIO_PIN_SET);  // lcd back light power on
		g_lcdEnable = 1;
  }
//  void main_lcdEnableStep3(void** params) {
//    lcd_init();
//    schedule_once(LCD_INIT_SCHEDULE_ID, main_lcdEnableStep4, 10, 0);
//  }
//  void main_lcdEnableStep2(void** params) {
//   // HAL_GPIO_WritePin(GPIOC,GPIO_PIN_10,GPIO_PIN_SET);
//    schedule_once(LCD_INIT_SCHEDULE_ID, main_lcdEnableStep3, 120, 0);
//  }
  void main_lcdEnableStep1(void** params) {
   // HAL_GPIO_WritePin(GPIOC,GPIO_PIN_10,GPIO_PIN_RESET);
    schedule_once(LCD_INIT_SCHEDULE_ID, main_lcdEnableStep4, 1, 0);
  }
  void main_lcdEnable(uint8_t enable) {
		g_lcdEnable = 1;
//		if (enable) {
//			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_11,GPIO_PIN_SET);
//			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_10,GPIO_PIN_SET);
//			schedule_once(LCD_INIT_SCHEDULE_ID, main_lcdEnableStep1, 1, 0);
//		} else {
//			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_11,GPIO_PIN_RESET);
//			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12,GPIO_PIN_RESET);
//			schedule_cancel(LCD_INIT_SCHEDULE_ID);
//		}
		HAL_GPIO_WritePin(GPIOF,GPIO_PIN_4,GPIO_PIN_SET);//power
		//schedule_once(LCD_INIT_SCHEDULE_ID, main_lcdEnableStep1, 1, 0);
  }
#endif
/******** ui functions *********/
#if 1
	void main_uiTimeTimer(void** params) {
		uint32_t now = time_getTime();
		if (now) {
			lcd_set32(time, now/60*60);
		}
	}
	void main_uiInit(void** params) {
//		lcd_set(settingsMenuSize, 8);
		
		lcd_set(wifiStatus, 0);
		//lcd_set(pin, 1);

		lcd_set(batteryBlend, 0);
		lcd_set(batteryIndex, 0);

		lcd_set(chargingAlpha, 127);
		
		char** spaces;
		if (string_split(g_deviceFirmwareVersion,' ',&spaces) == 3) {
			char** commas;
			if (string_split(spaces[1],'.',&commas) == 3) {
				fors(3) {
					int32_t value;
					if (number_fromDecString(commas[i], strlen(commas[i]), &value, 0)) {
						lcd_set(settingsVersion1 + i, value);
					}
				}
				fors(3) {
					free(commas[i]);
				}
			}
			fors(3) {
				free(spaces[i]);
			}
		}
		lcd_init();
//		lcd_set(settingsUpgradePercent, 0xff);
//		lcd_set(page, LCD_PAGE_main);
		
		lcd_set(theme, 2);
//		screen_fill(0,0,240,240,rgb(255,0,0)); 

		// lcd_set32(airUpdateTime, 1);
		schedule_repeat(0,main_uiTimeTimer,1000,0);
		main_uiTimeTimer(0);
	}
#endif
/******** setting functions *********/
#if 1
	static uint8_t esp8266AddCommand(wireless_key_t key, package_element_t* valueE, uint32_t time);
	
	const wireless_key_t g_settingsKeys[] = {
		wireless_key(command_data,settings_00),
		wireless_key(command_data,settings_01),
		wireless_key(command_data,settings_02),
		wireless_key(command_data,settings_03),
		wireless_key(command_data,settings_04),
		wireless_key(command_data,settings_05),
	};

	//uncomment 20190103 lai
	static void settingsGenerateCommand(int32_t index, int32_t value, uint32_t time) {
		esp8266AddCommand(g_settingsKeys[index], package_newInt8(value), time);
	}
	
	static void settingsUpdated(int8_t index, int8_t value, uint32_t delayed) {
		g_settingss_value[index] = value;
		uint32_t time = time_getTime();
		if (time) {
			time -= delayed;
		}
		powerChanged();
		g_settingss_time[index] = time;
		fpe_write(MAIN_FPE_BASE_settings + index, value);
		fpe_write(MAIN_FPE_BASE_settings + 0x100 + index, time);
	}
	
	static uint8_t settingsReceivedCommand(package_element_t* keyE, package_element_t* valueE, uint32_t time) {
		forta(wireless_key_t, g_settingsKeys) {
			if (wireless_keyEqual(keyE,v)) {
				if (time > g_settingss_time[i]) {
					uint32_t value = package_getInt(valueE);
					uint32_t now = time_getTime();
					settingsUpdated(i, value, now - time);
					//uiSettingsChanged();
				}
				return 1;
			}
		}
		return 0;
	}


//	static void settingsSubmited(void) {
////		int8_t menu = system_get(menuCursorIndex);
////		int8_t select = system_get(selectCursorIndex);
////		settings_set(0 + menu, select);
////		settingsGenerateCommand(menu, select, time_getTime());
//	}

	const static int8_t g_settingssDefaultSelecteds[] = {5,3,5,11,1,0};
	static void settingsInit(void** params) {
		fors (SYSTEM_MENU_PAGE_SELECTABLE_SIZE) {
			g_settingss_value[i] = fpe_readOr(MAIN_FPE_BASE_settings + i,g_settingssDefaultSelecteds[i]);
			g_settingss_time[i] = fpe_readOr(MAIN_FPE_BASE_settings + 0x100 + i,0);
			settingsGenerateCommand(i, g_settingss_value[i], g_settingss_time[i]);
		}
					
	}
#endif	



	/******************esp8266 related********************/
#if 1




//	static double  g_latitude=0.0;//Î³¶È-90~+90
//	static double  g_longitude=0.0;//¾­¶È-180~+180

	static char* g_pushAddress = 0;
	static uint16_t g_pushPort = 0;

	static uint32_t g_lastPushedTick = 0;
	static uint32_t g_lastRecevedTick = 0;
	static int32_t g_lastResponseCount = 0;
	static int32_t g_lastCommandCount = 0;
	static uint8_t g_waitForReceive = 0;

	static void wirelessResponse(wireless_t *wireless, package_element_t* key, package_element_t* requests, package_element_t* response) {
		// time
		g_lastResponseCount ++;
		if (wireless_keyEqual(key,wireless_key(request,time))) {
			// package_element_t* data = wireless_findValueByKey(response, wireless_key(request, data));
			package_element_t* data = response;
			if (data) {
				package_element_t* timestamp = wireless_findValueByKey(data, wireless_key(request_time, timestamp));
				if (timestamp) {
					uint32_t value = package_getInt(timestamp);
					if (value) {
						time_setTime(value);
						server_updated(time) = time_getTime();
					}
				}
			}
		}
		//weather
		// weather
		if (wireless_keyEqual(key,wireless_key(request,weather))) {
			// package_element_t* data = wireless_findValueByKey(response, wireless_key(request_weather, data));
			package_element_t* data = response;
			if (data) {
				uint8_t isWeatherData = 0, isAirData = 0;
				package_element_t* data2 = wireless_findValueByKey(data, wireless_key(request_weather, data));
				if (data2) {
					wireless_key_t keys[] = {
						wireless_key(data, pm2d5),
					};
					forta (wireless_key_t, keys) {
						package_element_t* vE = wireless_findValueByKey(data2, v);
						if (vE) {
//							if (i < 7) {
//								isAirData = 1;
								int16_t value;
								{
									intMax_t v;
									int8_t e;
									if (package_getNumber(vE, &v, &e)) {
										value = number_int(v, i == 6?e+3:e);
									} else {
										value = -1;
									}
								}
								lcd_set16(valueOutdoorPm2d5, value);
//							} 
						}
					}
				}
			}
		}
	}

	static void wirelessCommand(wireless_t *wireless, package_element_t* data, uint32_t delayed, uint8_t by) {
		g_lastCommandCount ++;
		package_element_t* key, *value;
		list_t* iter = package_getFirstPair(data, &key, &value);
		uint32_t now = time_getTime();
		while(iter) {
			if (key && value) {
				// settings
				uint8_t done = 0;
				if (!done) {
					done = settingsReceivedCommand(key, value, delayed == UINT32_MAX?0:now - delayed/1000);
				}
//				if (!done) {
//					done = uotaReceivedCommand(key, value);
//				}
//				if (!done) {
//					done = weatherReceivedCommand(key,value);
//				}
			}
			iter = package_getNextPair(iter, &key, &value);
		}
	}

	static void wirelessData(wireless_t *wireless, package_element_t* key, package_element_t* value) {
		// none
	}
	static void wirelessLineChanged(wireless_t* wireless, uint8_t type, uint16_t line) {
		fpe_write(MAIN_FPE_BASE_wireless+type, line);
	}

	static void wirelessAccepted(wireless_t *wireless, package_element_t* data, uint32_t delayed, uint8_t by) {
		// settings
	}

	static void wirelessRoute(wireless_t *wireless, char* address, uint16_t port) {
		free(g_pushAddress);
		g_pushAddress = string_duplicate(address, strlen(address));
		g_pushPort = port;
		// settings
	}

	static void wirelessInit(wireless_t *wireless) {
		if (!wireless_findValueByKey(wireless->uploadPackage, wireless_key(root,deviceId))) {
			char deviceId[40];
			if (getDeviceId(deviceId)) {
				wireless_setDeviceId(wireless, deviceId);
			}
		}
		wireless_setVersion(wireless, g_deviceHardwareVersion, g_deviceFirmwareVersion, g_deviceProtocol);
	}
	static wireless_t g_wireless = {
		wirelessResponse,
		wirelessCommand,
		wirelessData,
		wirelessLineChanged,
		wirelessAccepted,
		wirelessRoute,
		wirelessInit,
	};

	static uint8_t g_esp8266ReceiveBuffer[256] = {0};
	static uint8_t g_esp8266TransmitBuffer[512] = {0};
	static circularBuffer_t g_esp8266ReceiveCB = {g_esp8266ReceiveBuffer, sizeof(g_esp8266ReceiveBuffer),0,0};
	static circularBuffer_t g_esp8266TransmitCB = {g_esp8266TransmitBuffer, sizeof(g_esp8266TransmitBuffer),0,0};
	static stream_t g_esp8266ReceiveStream = {&g_esp8266ReceiveCB, 0, 0};
	static stream_t g_esp8266TransmitStream = {&g_esp8266TransmitCB, 0, 0};
	static esp8266_t g_esp8266 = {
		&g_esp8266TransmitStream,
		&g_esp8266ReceiveStream,
	};

//	static void esp8266Sending(esp8266_t* esp8266, wireless_socket_t* socket, uint8_t* buffer, int16_t size) {
//	}

	static void esp8266Reveived(esp8266_t* esp8266, wireless_socket_t* socket, uint8_t* buffer, int16_t size) {
		//string_trimS(&buffer,&size);
		//buffer[size] = 0;
		g_waitForReceive = 0;
		g_lastResponseCount = 0;
		g_lastCommandCount = 0;
		g_lastRecevedTick = tick_ms();
		esp8266_setStatus(ESP8266_STATUS_ok);
		esp8266_delayStatusChecker(0);
		if (!schedule_exists(MAIN_ESP8266_AUTO_RESET_SCHEDULE_ID))
		{
			schedule_once(MAIN_ESP8266_AUTO_RESET_SCHEDULE_ID, esp8266CheckForCrash, 1000, 0);
		}
		wireless_received(&g_wireless, buffer, size);
	}

	static uint8_t esp8266AddData(wireless_key_t key, package_element_t* valueE) {
		if (!valueE) {
			return 0;
		}
		package_element_t* keyE = wireless_new(key);
		if (!keyE || !wireless_addData(&g_wireless, keyE, valueE)) {
			package_delete(keyE);
			package_delete(valueE);
			return 0;
		}
		return 1;
	}
	// static uint8_t esp8266AddBytesData(wireless_key_t key, uint8_t* bytes, int16_t size) {
	// 	return esp8266AddData(key, package_newBytes(bytes, size));
	// }

	static uint8_t esp8266AddNumberData(wireless_key_t key, int32_t value, int8_t e) {
		package_element_t* valueE;
		if (e==0) {
			valueE = package_newInt32(value);
		} else {
			valueE = package_newDec32(value,e);
		}
		return esp8266AddData(key, valueE);
	}
//	static package_element_t* esp8266GetRequest(wireless_key_t key) {
//		return wireless_getRequest(&g_wireless, key);
//	}
	static int16_t esp8266GetRequestsCount(void) {
		return wireless_getRequestsCount(&g_wireless);
	}
//	static uint8_t esp8266AddRequest(wireless_key_t key, package_element_t* valueE) {
//		if (!valueE) {
//			return 0;
//		}
//		package_element_t* keyE = wireless_new(key);
//		if (!keyE || !wireless_addRequest(&g_wireless, keyE, valueE)) {
//			package_delete(keyE);
//			package_delete(valueE);
//			return 0;
//		}
//		return 1;
//	}
	static uint8_t esp8266AddCommand(wireless_key_t key, package_element_t* valueE, uint32_t time) {
		if (!valueE) {
			return 0;
		}
		package_element_t* keyE = wireless_new(key);
		if (!keyE || !wireless_addCommand(&g_wireless, keyE, valueE,time)) {
			package_delete(keyE);
			package_delete(valueE);
			return 0;
		}
		return 1;
	}
	#define REQUEST_PER_TIME 1
	#define shouldAddRequest() (esp8266GetRequestsCount() < REQUEST_PER_TIME)
//	static void serverDataRequest(void** params) {
//		uint32_t now = time_getTime();
//		if (shouldAddRequest() && (!now || now - server_updated(time) > 5 * 60 && now - server_fetched(time) > 5 * 60) && !esp8266GetRequest(wireless_key(request, time))) {
//			if (esp8266AddRequest(wireless_key(request, time), package_newObject())) {
//				server_fetched(time) = now;
//			}
//		}
//	}

	static uint32_t pushInterval(void) {
//		return settings_get(uploadInterval) * 5000 + 5000;
		return 5000;
	}

	static void serverPusher(void** params) {
		uint8_t pass = 0;
		uint32_t now = tick_ms();
		static uint16_t uploadLineGenerator;
		static uint8_t requesting = 0;
		static uint8_t firstTimeSend = 1;
		if (firstTimeSend == 1){
			pass = 1;
			firstTimeSend = 0;
		} else if (now - pushInterval() - g_lastPushedTick < INT32_MAX) {
			pass = 1;
		} else if (requesting && g_lastResponseCount > 0 && !g_waitForReceive && now - 200 - g_lastRecevedTick < INT32_MAX) {
			pass = 1;
		} else if (g_lastCommandCount > 0 && !g_waitForReceive && now - 300 - g_lastRecevedTick < INT32_MAX) {
			pass = 1;
		} else if (uploadLineGenerator!= g_wireless.uploadLineGenerator && !g_waitForReceive && now - 200 - g_lastRecevedTick < INT32_MAX) {
			pass = 1;
		}
		if (!pass) {
			return;
		}
		if (!esp8266_ready() || g_warmingTime > 1) {
			return;
		}

//		if((g_esp8266.wifiSsid) != NULL) {
//			esp8266AddCommand(wireless_key(command_data,ssid), package_newString((g_esp8266.wifiSsid)), 0);
//		}

//		}
//		if ((done & (1<<1)) == 0) {
//			if(g_esp8266Imsi[0] != 0xff
//				&& esp8266AddCommand(wireless_key(command_data,imsi), package_newBytes(g_esp8266Imsi, 15), 0)) {
//				done |= (1<<1);
//			}
//		}
//		if ((done & (1<<2)) == 0) {
//			if(esp8266AddCommand(wireless_key(command_data,ipLocate), package_newBoolean(0), 0)) {
//				done |= (1<<2);
//			}
//		}
		
//		if ((done & (1<<3)) == 0) {
//			if(	g_latitude && g_longitude&& esp8266AddCommand(wireless_key(command_data,latitude), package_newDouble(g_latitude), 0)&&	esp8266AddCommand(wireless_key(command_data,longitude), package_newDouble(g_longitude), 0)) {
//				done |= (1<<3);
//			}
//		}

		//serverDataRequest(0);
		requesting = esp8266GetRequestsCount() > 0;
		uploadLineGenerator = g_wireless.uploadLineGenerator;
		// static uint8_t ts = 0;
		// if (ts) {
		// 	return;
		// }
		// ts = 1;
		uint8_t* toSend;
//		if (enabled(sensor))
		if (1)			{
//			esp8266AddNumberData(wireless_key(data,pm2d5),sensor_get(pm2d5),-2);
//			esp8266AddNumberData(wireless_key(data,pm10),sensor_get(pm10),-2);
			esp8266AddNumberData(wireless_key(data,temperature),sensor_get(temperature),-2);
			esp8266AddNumberData(wireless_key(data,humidity),sensor_get(humidity),-2);
//			esp8266AddNumberData(wireless_key(data,pm1d0),sensor_get(pm1d0),-2);
//			#if CO2_SWITCH
//			esp8266AddNumberData(wireless_key(data,co2),sensor_get(co2),0);
//			#endif
//			#if CHEMICAL_SWITCH
//			esp8266AddNumberData(wireless_key(data,tvoc),sensor_get(tvoc),0);
//			#endif
			#if 1
			esp8266AddNumberData(wireless_key(data,ch2o),sensor_get(ch2o),-3);
			#endif
//			esp8266AddNumberData(wireless_key(data,windSpeed),sensor_get(windSpeed),0);
//			esp8266AddNumberData(wireless_key(data,windDirection),sensor_get(windDirection),0);
			
			esp8266AddNumberData(wireless_key(data,_interval),min(now - g_lastPushedTick, pushInterval()),-3);
		}
		int16_t size = wireless_fetchBytes(&g_wireless, time_getTime(), &toSend);
		if (size > 0) {
			// char* address = g_pushAddress?g_pushAddress:"123.57.56.40";
			char const* address = g_pushAddress?g_pushAddress:g_pushDefaultAddress;
			uint16_t port = g_pushPort?g_pushPort:g_pushDefaultPort;
			esp8266_send(1, address, port, toSend, size);
			free(toSend);
		}

		g_lastPushedTick = now;
		g_waitForReceive = 1;
  }
	#define MAIN_ESP8266_INFO_SCHEDULE_ID 0xfec18266


	static void esp8266CheckForCrash(void** params)
	{
		
		uint32_t now = tick_ms();
		if (now - 60000 - g_lastRecevedTick < (UINT32_MAX>>1))
		{
			//if (g_esp8266.status != ESP8266_STATUS_smarting)
			{
				esp8266_reset(0);
			}
			g_lastRecevedTick = now;
		}
		schedule_once(MAIN_ESP8266_AUTO_RESET_SCHEDULE_ID, esp8266CheckForCrash, 1000, 0);
	}

//	void esp8266_smartFinished(esp8266_t* esp8266) {
//		lcd_set(LCD_DATA_wifiStatus,esp8266->status);
//	}
	static void esp8266IpChanged(esp8266_t* esp8266)
	{
		esp8266AddCommand(wireless_key(command_data,ssid), package_newString((g_esp8266.wifiSsid)), 0); 
	}
	static void esp8266SmartFinished(esp8266_t* esp8266)
	{//WIRELESS_KEY_command_data_pairing

		{
			
			//esp8266AddCommand(wireless_key(command_data,ssid), package_newString((g_esp8266.wifiSsid)), 0); 
			esp8266AddCommand(wireless_key(command_data,pairing), package_newBoolean(1), 0);
		}
		//esp8266AddCommand();
	}
	static void esp8266Init(void** params) {
		//uart2_switch2Esp8266();
		UARTswitchChannel(1);
		schedule_cancel(SCHEDULE_OUT_FIX_ID);
		g_esp8266.sendingFunction = 0;
		g_esp8266.receivedFunction = esp8266Reveived;
		g_esp8266.smartLinkFunction = esp8266SmartFinished;
		g_esp8266.ipChangedFunction = esp8266IpChanged;
		g_esp8266.status = ESP8266_STATUS_starting;
		g_esp8266.wifiSsid = NULL;
    esp8266_init(&g_esp8266);
		
//		fpe_readBytes(MAIN_FPE_BASE_imei,g_esp8266Imei,15);
//		fpe_write(MAIN_FPE_BASE_GPS_LATITUDE,g_latitude*1000000);//test
//		fpe_write(MAIN_FPE_BASE_GPS_LONGITUDE,g_longitude*1000000);
//		g_latitude = fpe_readOr(MAIN_FPE_BASE_GPS_LATITUDE,0)/1000000.0;
//		g_longitude = fpe_readOr(MAIN_FPE_BASE_GPS_LONGITUDE,0)/1000000.0;
		
		g_wireless.downloadLine = fpe_readOr(MAIN_FPE_BASE_wireless + WIRELESS_LINE_TYPE_download,0);
		g_wireless.uploadLine = fpe_readOr(MAIN_FPE_BASE_wireless + WIRELESS_LINE_TYPE_upload,0);
		g_wireless.uploadLineGenerator = fpe_readOr(MAIN_FPE_BASE_wireless + WIRELESS_LINE_TYPE_uploadGenerator,0);
		wireless_init(&g_wireless);

	
//		schedule_once(MAIN_ESP8266_AUTO_RESET_SCHEDULE_ID, esp8266CheckForCrash, 1000, 0);
//		schedule_repeat(STATUS2LED_SCHEDULE_ID, status2LED, 1000, 0);
  }
//	static void esp8266Deinit(void** params) {
//		if (schedule_exists(MAIN_ESP8266_AUTO_RESET_SCHEDULE_ID))
//		{
//			schedule_cancel(MAIN_ESP8266_AUTO_RESET_SCHEDULE_ID);
//		}
//		esp8266_deinit(&g_esp8266);
//	}
	
#endif
	
	

uint8_t isButtonPressed(uint32_t id)
{
	switch (id)
	{
		case BUTTON_DOME:
			return HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_4);
//		case BUTTON_TOUCH:
//			return !HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_2);
		default:
			return 0;
	}
}


  void onPowerButtonReleased (void** params) {
		if (system_get(page) == LCD_PAGE_prestart) {
			powerOff();
		}
	}
	
void onPowerButtonHold(void** params)
{
  	interacted();
		switch (system_get(page)) {
			case LCD_PAGE_start:
				pageToShut();
				break;
			case LCD_PAGE_main:
				pageToShut();
				break;
			case LCD_PAGE_settings_qrcode:
	  		//system_set(wifiStatus,1);
				//esp8266_smartStop();//2016-3-18.
				if (esp8266_getStatus() == ESP8266_STATUS_smarting)
				{
					esp8266_smartStop();
				}
				if (esp8266_getStatus() >= ESP8266_STATUS_initOK)
				{
					schedule_reset(MAIN_ESP8266_AUTO_RESET_SCHEDULE_ID, 300000);
					esp8266_smart(300000);	
				}
				break;
			case LCD_PAGE_battery:
//				system_set(holding, 0);
				pageToStart();
				break;
//			case LCD_PAGE_shut:
//				break;
			case LCD_PAGE_prestart:
			//	system_set(holding, 0);
				main_lcdEnable(1);
				main_uiInit(0);
				pageToStart();
				break;
//			case LCD_PAGE_screen_off:
//				screenWakeUp();

			default:
				break;
		}
  	uiChanged();	

}


void onMode2ButtonPressed(void** params)
{
  	interacted();
		switch (system_get(page)) {
			case LCD_PAGE_settings_qrcode:
	  		//system_set(wifiStatus,1);
				//esp8266_smartStop();//2016-3-18.
				if (esp8266_getStatus() == ESP8266_STATUS_ok)
				{
					esp8266SmartFinished(&g_esp8266);
				}
				break;
			default:
				break;
		}
  	uiChanged();	
}
	/******** events functions *********/
#if 1
  void main_eventWatcher(void** params) {
    uint8_t page = system_get(page);
    uint32_t animationTime = tick_ms() - system_get32(animationStartTime);
    if (page == LCD_PAGE_start) {
			if (lcd_get(ch2oPreparing) == 0 || animationTime >= 16000)
			{
				
        system_set(page,LCD_PAGE_main);
        uiChanged();
      }
			if (lcd_get(ch2oPreparing) == 1 && animationTime >= 10000)
			{
				if (!schedule_exists(SCHEDULE_ID_ESP8266_INIT) && g_usbUartDetected == 0 && g_esp8266.receivedFunction == NULL)
				{
					schedule_once(SCHEDULE_ID_ESP8266_INIT, esp8266Init, 0, 0);	
				}					
			}
    }
		if (page == LCD_PAGE_shut) {
      //if (animationTime > 2000) 
				{
				powerOff();
      }
    }
		if (page == LCD_PAGE_low_power) {
      if (animationTime > 1000) {
				powerOff();
      }
    }
		
		if (!g_main_is_charging && system_get(page) == LCD_PAGE_battery) {
			powerOff();
		}
				
		if (system_get(page) > LCD_PAGE_animate) {
			if (g_main_is_charging)
			{
				interacted();
			}
			int32_t now = tick_ms();
			const int32_t timeByIndex [] = {30, 10*60, 30*60, 60*60, 24*60*60};
			int32_t interactTime = system_get32(lastInteractTime);
			if (system_get(page) != LCD_PAGE_battery &&\
			settings_get(powerOffTime) != 5 && now - (interactTime + timeByIndex[settings_get(powerOffTime)] * 1000 ) > 0) {
				pageToShut();
			}
//			if (system_get(page) != LCD_PAGE_screen_off && \
//			settings_get(screenOffTime) != 5 && now - (interactTime + timeByIndex[settings_get(screenOffTime)] * 1000 ) > 0) {
//				screenSleep();
//			}
//			if (!system_get(holding) && \
//			settings_get(holdTime) != 5 && now - (interactTime + timeByIndex[settings_get(holdTime)] * 1000 ) > 0) {
//        system_set(holding,1);
//				powerChanged();
//		  	uiChanged();
//			}
		}
    if (g_main_uiChanged) {
      thread_execute(0,g_main_threadIds[MAIN_THREAD_ID_lcdUpdate]);
    }
//    if (g_main_powerChanged) {
//      updatePower();
//    }
  }
#endif
/******** butons functions *********/	
#if 1
 void onModeButtonPressed (void** params) {
  	interacted();
  	switch (system_get(page)) {
  		case LCD_PAGE_start:
				//g_warmingTime = 0;
				if (!schedule_exists(SCHEDULE_ID_ESP8266_INIT) && g_usbUartDetected == 0 && esp8266_getStatus() <= ESP8266_STATUS_starting && g_esp8266.receivedFunction == NULL)
				{
					schedule_once(SCHEDULE_ID_ESP8266_INIT, esp8266Init, 0, 0);	
				}
				system_set(page,LCD_PAGE_main);
  			uiChanged();			
  			break;
  		case LCD_PAGE_main:
//  			system_set(page,LCD_PAGE_settings_qrcode);
//				generateQRBuffer();
//  			uiChanged();
  			system_set(page,LCD_PAGE_all);
  			uiChanged();
			
  			break;
  		case LCD_PAGE_all:
  			system_set(page,LCD_PAGE_pm2d5);
  			uiChanged();
  			break;
  		case LCD_PAGE_pm2d5:
  			system_set(page,LCD_PAGE_settings_qrcode);
				generateQRBuffer();
  			uiChanged();
  			break;

  		case LCD_PAGE_settings_qrcode:
				system_set(page,LCD_PAGE_main);
  			uiChanged();
  			break;
//  		case LCD_PAGE_settings_info:
//  			break;
  		case LCD_PAGE_battery:
				if (!schedule_exists(SCHEDULE_ID_ESP8266_INIT) && g_usbUartDetected == 0 && esp8266_getStatus() <= ESP8266_STATUS_starting && g_esp8266.receivedFunction == NULL)
				{
					schedule_once(SCHEDULE_ID_ESP8266_INIT, esp8266Init, 0, 0);	
				}				
				pageToStart();
				//lcd_set(chargingAlpha, lcd_get(chargingAlpha) + 64);
  			//uiChanged();
  			break;
//  		case LCD_PAGE_shut:
			case LCD_PAGE_prestart:
							main_lcdEnable(1);
			main_uiInit(0);
				pageToStart();
				break;
//			case LCD_PAGE_screen_off:
////				screenWakeUp();
//				break;
  		default:
  			break;
  	}
  }
#endif
	static void hw_deinit(void)
{
	
//	dust_deinit();
//	#if AIRJ_LCD_DISPLAY
//	lcd_deinit();
//	#endif
	screen_powerOff();
	while (HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_4));
	esp8266_deinit(&g_esp8266);
	sensorPowerOff(0);
	HAL_GPIO_WritePin(GPIOF,GPIO_PIN_7,GPIO_PIN_RESET); // alx345 power off
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_RESET); // ch2o power off
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_RESET); // th power off
//	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_RESET); // battery charge enable
	
//	HAL_GPIO_DeInit(GPIOB,GPIO_PIN_3);
	
	HAL_ADC_DeInit(&hadc);//all adc
	HAL_UART_DeInit(&huart1);//iap
	HAL_UART_DeInit(&huart2);//esp8266
	HAL_I2C_DeInit(&hi2c2);//th
	HAL_I2C_DeInit(&hi2c1);//alx345
	HAL_TIM_Base_DeInit(&htim1);//tick
	
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_RESET); // dc-dc off
	
	
	__DMA1_CLK_DISABLE();
  __GPIOA_CLK_DISABLE();
  __GPIOB_CLK_DISABLE();
  __GPIOC_CLK_DISABLE();
  __GPIOD_CLK_DISABLE();
  __GPIOF_CLK_DISABLE();
	
	
}
	static void powerOff(void) {
		if (g_main_is_charging==1) {
		} else {
			//doPowerOffStandBy();
			enterSleep();
			// testPowerOff();
		}
//		esp8266Deinit(0);
		system_set(page, LCD_PAGE_battery);
		powerChanged();
		uiChanged();
	}	
void enterSleep(void)
{
	
	hw_deinit();
	
	HAL_Delay(100);
	{
		GPIO_InitTypeDef GPIO_InitStruct;
		__GPIOB_CLK_ENABLE();
		__SYSCFG_CLK_ENABLE();

//		GPIO_InitStruct.Pin = GPIO_PIN_4 |;
//		GPIO_InitStruct.Pull = GPIO_NOPULL;
//		GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
//		GPIO_InitStruct.Mode = GPIO_MODE_EVT_FALLING;
//		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = GPIO_PIN_3 | GPIO_PIN_4;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
		GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
		
	/* Enable and set Button EXTI Interrupt to the lowest priority */
		HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0x02, 0x00);
		HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
		HAL_NVIC_SetPriority(EXTI2_3_IRQn, 0x02, 0x00);
		HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);
		__GPIOB_CLK_DISABLE();
	}
//	g_main_status = STATUS_POWEROFF;
//	fpe_write(0x0100,g_main_status);
	HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
	//StandbyMode_Measure();
	
	HAL_NVIC_SystemReset();
	
}
/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */
//	EnterBootLoader();

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
//  MX_GPIO_Init();
//  MX_DMA_Init();
//  MX_ADC_Init();
//  MX_I2C1_Init();
//  MX_I2C2_Init();
//  MX_IWDG_Init();
//  MX_SPI2_Init();
//  MX_TIM1_Init();
//  MX_USART1_UART_Init();
//  MX_USART2_UART_Init();

  /* USER CODE BEGIN 2 */


	if (__HAL_PWR_GET_FLAG(PWR_FLAG_SB) != RESET)
  {
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
  }
	g_isIniting = 1;
	hw_init();
	sensorPowerOn(0);


	UARTswitchChannel(0);
	//memset(g_uart2Buffer,UINT8_MAX-1,sizeof(g_uart2Buffer));
//	HAL_UART_Receive_DMA(&huart1, (uint8_t *)g_uart2Buffer, UART2_BUFFER_SIZE);
	fpe_init(62,63);	
	loadFlashData();
	 
	tick_init();
	
	schedule_init();
	thread_init(0,tick_ms);
//	g_main_console = console_init(&huart1,g_main_consoleBuffer,sizeof(g_main_consoleBuffer));
//	console_addListener(g_main_console,"cal",newCalibration,0);

//	lcd_loadData();	
// button
	button_init(BUTTON_DOME,isButtonPressed);
//	button_init(BUTTON_TOUCH,isButtonPressed);
	

	button_addListener(BUTTON_DOME,BUTTON_EVENT_RELEASED,onPowerButtonReleased,0);	
	button_addListener(BUTTON_DOME,BUTTON_EVENT_HOLD,onPowerButtonHold,0);
	button_addListener(BUTTON_DOME,BUTTON_EVENT_CLICKED_NO_NEXT,onModeButtonPressed,0);
	button_addListener(BUTTON_DOME,BUTTON_EVENT_DOUBLE_CLICKED,onMode2ButtonPressed,0);
//	button_addListener(BUTTON_TOUCH,BUTTON_STATUS_CLICKED_NO_NEXT,onTouchButtonPressed,0);
//	button_addListener(BUTTON_TOUCH,BUTTON_STATUS_HOLD,onTouchButtonPressed,0);
	

	hts221_init();	
	//* thread
	g_main_threadIds[MAIN_THREAD_ID_lcdUpdate] = thread_quickAdd(0,main_lcdUpdate,20,0);
	thread_quickAdd(0,schedule_run,1,0);
	thread_quickAdd(0,main_eventWatcher,20,0);
//	thread_quickAdd(0,console_run,10,0,0);
	thread_quickAdd(0,button_run,1,0);
	thread_quickAdd(0,serverPusher,50,0);
	thread_quickAdd(0,stream_run,1,0);
//	thread_quickAdd(0,adxl345_update,50,0);
	 
	thread_quickAdd(0,battery_update,250,0);
	//thread_quickAdd(0,chargeCurrent_update,1000,1,0);
	thread_quickAdd(0,ch2o_update,1000,0);

	thread_quickAdd(0,th_update,1000,0);
	thread_quickAdd(0,dust_update,1000,0);

	schedule_repeat(MAIN_BATTERY_SCHEDULE_ID, main_batteryTimer, 500, 0);
	battery_update(0);
	battery_update(0);
	battery_update(0);
	
	schedule_reset(MAIN_BATTERY_SCHEDULE_ID,0);
//	#if WIFI_UDP_MODE
//	thread_quickAdd(0,wifi_response,100,0);
//	thread_quickAdd(0,mainThread,1000,0);
//	thread_quickAdd(0,esp8266_run,10,0);
//	thread_quickAdd(0,updateWifiStatus,1000,0);
//	#else
//	thread_quickAdd(0,esp8266_run,10,1,0);
//	thread_quickAdd(0,wifi_deal,g_main_wifi_interval,1,0);
//	thread_quickAdd(0,updateWifiStatus,1000,1,0);
//	#endif
	//* tick

//	tick_add(console_tick,100);
	tick_add(button_tick,10,0);
	tick_add(tempFixTick,3000,0);
//	#if CO2_ENABLE
	tick_add(co2_tick,1,0);
//	#endif
	tick_start();
	
	pm2d5_init();



	HAL_UART_Receive_DMA(&huart1, (uint8_t *)g_usb_configBuffer, 8);

	schedule_repeat(SCHEDULE_OUT_FIX_ID,output_fix,200,0);
	HAL_UART_Receive_DMA(&huart1, (uint8_t *)g_usb_configBuffer, 8);
//	if (!g_main_is_charging)
//	{
//		schedule_once(SCHEDULE_ID_ESP8266_INIT, esp8266Init, 1000, 0);
////		esp8266Init(0);
////		#if 0
////		esp8266_init(&huart1);
////		esp8266_setSmartCallback(wifiSmartLinkStarted);
////		char* mac = esp8266_getMAC();
////		#endif

//	}

		main_ch2oStartWarm();
//	getID_PWD();
	
//	char str[15] = "at+outtype=4\r\n";

	
	UART_config_B2("at+outtype=4\r\n");
	UART_config_B2("at+upper=0\r\n");
	
//	HAL_Delay(200);
//	UART_config_B2("at+sensitivity=4\r\n");
//#if COMPRESS_BMP
//#if COMPRESS_BMP
//		__IO char buffer[1100] ={0};
//		__IO uint16_t size = 0;
//		UARTswitchChannel(0);
//		lcd_Compress(buffer,&size)
//		HAL_UART_Transmit(&huart1,buffer,size,300);
//#endif 
	settings_set(powerOffTime,2);
		if (isButtonPressed(BUTTON_DOME)) {
			system_set(page, LCD_PAGE_prestart);
		}
		else {
				main_lcdEnable(1);
				main_uiInit(0);
			
		//	pageToStart();
			if (g_main_is_charging) {
				system_set(page, LCD_PAGE_battery);
			} else {
				system_set(page, LCD_PAGE_main);
			}
		}
	powerChanged();
	/* USER CODE END 2 */

  /* USER CODE BEGIN 3 */
  /* Infinite loop */
  while (1)
  {
		thread_run(0);
  }
  /* USER CODE END 3 */

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSI14
                              |RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSI14State = RCC_HSI14_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.HSI14CalibrationValue = 16;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL12;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1);

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_I2C1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

  __SYSCFG_CLK_ENABLE();

}

/* ADC init function */
void MX_ADC_Init(void)
{

  ADC_ChannelConfTypeDef sConfig;

    /**Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
    */
  hadc.Instance = ADC1;
  hadc.Init.ClockPrescaler = ADC_CLOCK_ASYNC;
  hadc.Init.Resolution = ADC_RESOLUTION12b;
  hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;
  hadc.Init.EOCSelection = EOC_SINGLE_CONV;
  hadc.Init.LowPowerAutoWait = DISABLE;
  hadc.Init.LowPowerAutoPowerOff = DISABLE;
  hadc.Init.ContinuousConvMode = ENABLE;
  hadc.Init.DiscontinuousConvMode = DISABLE;
  hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc.Init.DMAContinuousRequests = ENABLE;
  hadc.Init.Overrun = OVR_DATA_PRESERVED;
  HAL_ADC_Init(&hadc);

    /**Configure for the selected ADC regular channel to be converted. 
    */
  sConfig.Channel = ADC_CHANNEL_5;
 // sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;
  HAL_ADC_ConfigChannel(&hadc, &sConfig);

}

/* I2C1 init function */
void MX_I2C1_Init(void)
{

  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x2000090E;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLED;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLED;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLED;
  HAL_I2C_Init(&hi2c1);

    /**Configure Analogue filter 
    */
//  HAL_I2CEx_AnalogFilter_Config(&hi2c1, I2C_ANALOGFILTER_ENABLED);

}

/* I2C2 init function */
void MX_I2C2_Init(void)
{

  hi2c2.Instance = I2C2;
  hi2c2.Init.Timing = 0x2000090E;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLED;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLED;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLED;
  HAL_I2C_Init(&hi2c2);

    /**Configure Analogue filter 
    */
//  HAL_I2CEx_AnalogFilter_Config(&hi2c2, I2C_ANALOGFILTER_ENABLED);

}

/* IWDG init function */
//void MX_IWDG_Init(void)
//{

//  hiwdg.Instance = IWDG;
//  hiwdg.Init.Prescaler = IWDG_PRESCALER_4;
//  hiwdg.Init.Window = 4095;
//  hiwdg.Init.Reload = 4095;
//  HAL_IWDG_Init(&hiwdg);

//}

/* SPI2 init function */
//void MX_SPI2_Init(void)
//{

//  hspi2.Instance = SPI2;
//  hspi2.Init.Mode = SPI_MODE_MASTER;
//  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
//  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
//  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
//  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
//  hspi2.Init.NSS = SPI_NSS_SOFT;
//  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
//  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
//  hspi2.Init.TIMode = SPI_TIMODE_DISABLED;
//  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;
//  hspi2.Init.NSSPMode = SPI_NSS_PULSE_DISABLED;
//  HAL_SPI_Init(&hspi2);

//}

/* TIM1 init function */
void MX_TIM1_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 47;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 99;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  HAL_TIM_Base_Init(&htim1);

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig);

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig);

}

/* USART1 init function */
void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONEBIT_SAMPLING_DISABLED ;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  HAL_UART_Init(&huart1);

}

/* USART2 init function */
void MX_USART2_UART_Init(void)
{

  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONEBIT_SAMPLING_DISABLED ;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  HAL_UART_Init(&huart2);

}

/** 
  * Enable DMA controller clock
  */
void MX_DMA_Init(void) 
{
  /* DMA controller clock enable */
  __DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __GPIOC_CLK_ENABLE();
  __GPIOF_CLK_ENABLE();
  __GPIOA_CLK_ENABLE();
  __GPIOB_CLK_ENABLE();
  __GPIOD_CLK_ENABLE();

  /*Configure GPIO pins : PC13 PC14 PC15 PC0 
                           PC1 PC2 PC3 PC4 
                           PC5 PC6 PC7 PC8 
                           PC9 PC10 PC11 PC12 */
	GPIO_InitStruct.Pin = GPIO_PIN_All;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
//  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PF0 PF1 PF4 PF5 
                           PF6 PF7 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5 
                          |GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
//  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 PA1 PA4 PA7 
                           PA8 PA11 PA12  */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_7 
                          |GPIO_PIN_8|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
//  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB3 PB5 PB6 
                           PB7 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_5|GPIO_PIN_6 
                          |GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
 // GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
#if !NEW_M2_PCB
  /*Configure GPIO pin : PB1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
#endif	
#if NEW_M2_PCB
  /*Configure GPIO pin : PB2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
#endif	
  /*Configure GPIO pin : PD2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : PB4 */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

//void EnterBootLoader()
//{
//	uint32_t addressSystemMerory = 0x1fffec00;
//	void((*BootLoaderEntry)()) = (void((*)()))(*((__IO uint32_t *)(addressSystemMerory+4)));
//	__set_MSP(*(__IO uint32_t*)addressSystemMerory);
//	(*BootLoaderEntry)();
//}

//void setAllLevelColor(void)
//{
//setLevelColor(GAS_PM25,g_main_PM2_5Value,uint16_t *color,uint8_t *level)

//}
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* h)
{
	HAL_ADC_Stop(&hadc);
	g_adcConvertDone = 1;
}
/* USER CODE END 4 */

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
/* User can add his own implementation to report the file name and line number,
ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
