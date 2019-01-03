#include "stm32f0xx_hal.h"
#include "frameworks/time.h"
//RTC_TypeDef* rtc = RTC1;
//static void register2time(time_t* time, uint32_t* tr, uint32_t* dr) {
//	time->hour = ((*tr >> 20) & 0x03) * 10 + ((*tr >> 16) & 0x0f);
//	time->minute = ((*tr >> 12) & 0x07) * 10 + ((*tr >> 8) & 0x0f);
//	time->second = ((*tr >> 4) & 0x07) * 10 + (*tr & 0x0f);

//	time->year = 30 + ((*dr >> 20) & 0x0f) * 10 + ((*dr >> 16) & 0x0f) - 1;
//	time->month = ((*dr >> 12) & 0x01) * 10 + ((*dr >> 8) & 0x0f) - 1;
//	time->day = ((*dr >> 4) & 0x03) * 10 + (*dr & 0x0f) - 1;

//	time->weekday = ((*dr >> 13) & 0x07);
//}

//static void time2register(time_t* time, uint32_t* tr, uint32_t* dr) {
//	uint32_t ht, hu, mnt, mnu, st, su, yt, yu, wdu, mt, mu, dt, du;
//	ht = time->hour / 10;
//	hu = time->hour % 10;
//	mnt = time->minute / 10;
//	mnu = time->minute % 10;
//	st = time->second / 10;
//	su = time->second % 10;
//	uint32_t year = (time->year + 100 - 30) % 100;
//	yt = year / 10;
//	yu = year % 10;
//	mt = (time->month + 1) / 10;
//	mu = (time->month + 1) % 10;
//	dt = (time->day + 1) / 10;
//	du = (time->day + 1) % 10;
//	// wdu = time->weekday + 1;
//	wdu = 0;

//	*tr = ((ht & 0x03) << 20) | ((hu & 0x0f) << 16) | ((mnt & 0x07) << 12) | ((mnu & 0x0f) << 8) | ((st & 0x07) << 4) | (su & 0x0f);
//	*dr = ((yt & 0x0f) << 20) | ((yu & 0x0f) << 16) | ((wdu & 0x07) << 13) | ((mt & 0x01) << 12) | ((mu & 0x0f) << 8) | ((dt & 0x03) << 4) | (du & 0x0f);
//}

//static uint32_t g_tick;
//uint32_t time_hw_past(void) {
//	return (HAL_GetTick() - g_tick) / 1000;
//}

//void time_hw_reset(void) {
//	g_tick = HAL_GetTick();
//}

//static RTC_HandleTypeDef hrtc1 = {0};

//// RTC_HandleTypeDef hrtc1;

///* RTC init function */
//static void rtcInit(void) {
//  hrtc1.Instance = RTC;
//  hrtc1.Init.HourFormat = RTC_HOURFORMAT_24;
//  hrtc1.Init.AsynchPrediv = 99;
//  hrtc1.Init.SynchPrediv = 399; //310
//  hrtc1.Init.OutPut = RTC_OUTPUT_DISABLE;
//  hrtc1.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
//  hrtc1.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
//}
//static void rtcEnable(void) {
//	HAL_RTC_Init(&hrtc1);
//}
//static void toRegister(time_t *time){
//	if (hrtc1.Instance == 0) {
//		rtcInit();
//	}
//	RTC_TimeTypeDef sTime;
//	RTC_DateTypeDef sDate;
//	sTime.Hours = RTC_ByteToBcd2(time->hour);
//	sTime.Minutes = RTC_ByteToBcd2(time->minute);
//	sTime.Seconds = RTC_ByteToBcd2(time->second);
//	sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
//	sTime.StoreOperation = RTC_STOREOPERATION_RESET;
//	if (HAL_RTC_SetTime(&hrtc1, &sTime, RTC_FORMAT_BCD) != HAL_OK)
//	{
//		return;
//	}
//	sDate.WeekDay = time->weekday;
//	if(time->weekday == 0)
//	{
//		sDate.WeekDay = RTC_WEEKDAY_SUNDAY;
//	}
//	sDate.Month = RTC_ByteToBcd2(time->month);
//	sDate.Date = RTC_ByteToBcd2(time->day);
//	sDate.Year = RTC_ByteToBcd2((time->year+70)%100);

//	if (HAL_RTC_SetDate(&hrtc1, &sDate, RTC_FORMAT_BCD) != HAL_OK)
//	{
//		return;
//	}
//}

//static void fromRegister(time_t *time){
//	if (hrtc1.Instance == 0) {
//		rtcInit();
//		rtcEnable(); //TODO 4T
//	}

//	RTC_DateTypeDef sdatestructureget;
//	RTC_TimeTypeDef stimestructureget;

//	HAL_RTC_GetTime(&hrtc1, &stimestructureget, RTC_FORMAT_BIN);
//	HAL_RTC_GetDate(&hrtc1, &sdatestructureget, RTC_FORMAT_BIN);
//	static uint8_t enabled;
//	if (!enabled && sdatestructureget.Year == 0) {
//		rtcEnable();
//		enabled = 1;
//		fromRegister(time);
//		return;
//	}
//	time->year = 30+sdatestructureget.Year;
//	time->month = sdatestructureget.Month;
//	time->day = sdatestructureget.Date;
//	time->weekday = sdatestructureget.WeekDay;
//	if(sdatestructureget.WeekDay == 7)
//	{
//		time->weekday =0;
//	}
//	time->hour = stimestructureget.Hours;
//	time->minute = stimestructureget.Minutes;
//	time->second = stimestructureget.Seconds;
//}

//void time_hw_setTime(uint32_t timestamp) {
//	time_t time;
//	time_fromSecond(timestamp, &time);
//	toRegister(&time);
//}
//uint32_t time_hw_getTime(void) {
//	time_t time;
//	fromRegister(&time);
//	if (time.year == 30) {
//		return 0;
//	}
//	return time_toSecond(&time);
//}
