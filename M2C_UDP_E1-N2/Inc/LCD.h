#ifndef LCD_H__
#define LCD_H__
#include "stdint.h"

enum lcd_page_t {
	LCD_PAGE_start = 0,
	LCD_PAGE_shut,
	LCD_PAGE_prestart,
	LCD_PAGE_low_power,
	LCD_PAGE_animate,
	LCD_PAGE_battery,
//	LCD_PAGE_screen_off,
//	LCD_PAGE_status,
	LCD_PAGE_main, /// norm pages
	LCD_PAGE_all, /// norm pages
	LCD_PAGE_pm2d5,
//	LCD_PAGE_air,
//	LCD_PAGE_weather,
//	LCD_PAGE_dust,
//	LCD_PAGE_history,
//	LCD_PAGE_settings_menu,
//	LCD_PAGE_settings_select,
	LCD_PAGE_settings_qrcode,
//	LCD_PAGE_settings_info,
	LCD_PAGE_SIZE,
};

enum lcd_data_t {
  // common
  LCD_DATA_page,
  LCD_DATA_theme,
  LCD_DATA_direction,
  LCD_DATA_time,
  LCD_DATA_time1,
  LCD_DATA_time2,
  LCD_DATA_time3,
  LCD_DATA_animationStartTime,
  LCD_DATA_animationStartTime1,
  LCD_DATA_animationStartTime2,
  LCD_DATA_animationStartTime3,
	// page title 10
//  LCD_DATA_uploading,
//  LCD_DATA_downloading,
	LCD_DATA_wifiStatus,
  LCD_DATA_batteryIndex,
	LCD_DATA_batteryRealIndex,
  LCD_DATA_batteryCharging,
  LCD_DATA_batteryBlend,
//  LCD_DATA_holding,

  // page main 18
	LCD_DATA_ch2oPreparing,
	LCD_DATA_ch2oPreTime,
	LCD_DATA_valueCh2o,
	LCD_DATA_valueCh2o1,
	
	LCD_DATA_valueCo2,
	LCD_DATA_valueCo21,
 
  LCD_DATA_valueTemperature,
  LCD_DATA_valueTemperature1,
  LCD_DATA_valueHumidity,
  LCD_DATA_valueHumidity1,
  LCD_DATA_valuePm2d5,
  LCD_DATA_valuePm2d51,
  LCD_DATA_valuePm2d52,
  LCD_DATA_valuePm2d53,
  // page air 38
  LCD_DATA_valueOutdoorPm2d5,
  LCD_DATA_valueOutdoorPm2d51,


  // settings
//  LCD_DATA_settingsMenuCursorIndex,
//  LCD_DATA_settingsMenuSize,
//  LCD_DATA_settingsSelectCursorIndex,
//  LCD_DATA_settingsSelectSelectedIndex,
//  LCD_DATA_settingsSelectSize,
  LCD_DATA_settingsQrBuffer,
  LCD_DATA_settingsQrBuffer1,
  LCD_DATA_settingsQrBuffer2,
  LCD_DATA_settingsQrBuffer3,
  LCD_DATA_settingsVersion1,
  LCD_DATA_settingsVersion2,
  LCD_DATA_settingsVersion3,
//  LCD_DATA_settingsUpgradePercent,
//  LCD_DATA_settingsImsi,
//  LCD_DATA_settingsImsi1,
//  LCD_DATA_settingsImsi2,
//  LCD_DATA_settingsImsi3,
  LCD_DATA_settingsDeviceId,
  LCD_DATA_settingsDeviceId1,
  LCD_DATA_settingsDeviceId2,
  LCD_DATA_settingsDeviceId3,
	// charging
	LCD_DATA_chargingAlpha,
  LCD_DATA_SIZE,
};

typedef struct
{
	uint8_t hour;
	uint8_t minute;
	uint8_t day;
	uint8_t month;
	uint16_t year;
}timeDate_t;
typedef struct
{
	uint8_t battery;
	uint8_t isWifiConnect;
	uint8_t temperature;
	uint8_t humidity;
	uint8_t level;
	uint16_t hcho;
	uint16_t co2;
	uint32_t pm2d5;
	timeDate_t time;
}homePageData_t;
extern uint8_t g_lcd_data_buffer[LCD_DATA_SIZE];
static inline void buffer_set(uint8_t* buffer, uint16_t k, int32_t value, int8_t size) {
	for (int8_t i = 0; i < size; i++) {
		buffer[k+i] = (value>>(i<<3)) & 0xff;
	}
}
static inline int32_t buffer_get(uint8_t* buffer, uint16_t k, int8_t size) {
	int32_t toRet = 0;
	uint8_t sign = 0;
	for (int8_t i = 0; i < size; i++) {
		sign = buffer[k+i];
		toRet |= sign<<(i<<3);
		sign >>= 7;
	}
	if (sign) {
		toRet -= (1<<(size<<3));
	}
	return toRet;
}
#define lcd_set(k,v) g_lcd_data_buffer[LCD_DATA_##k] = (v)
#define lcd_set32(k,v) buffer_set(g_lcd_data_buffer,LCD_DATA_##k,(v),4)
#define lcd_set16(k,v) buffer_set(g_lcd_data_buffer,LCD_DATA_##k,(v),2)
#define lcd_get(k) (g_lcd_data_buffer[LCD_DATA_##k])
#define lcd_get32(k) (buffer_get(g_lcd_data_buffer,LCD_DATA_##k,4))
#define lcd_get16(k) (buffer_get(g_lcd_data_buffer,LCD_DATA_##k,2))

void lcd_init(void);
void lcd_update(void);
uint8_t lcd_setRotation(uint16_t degree);
#endif
