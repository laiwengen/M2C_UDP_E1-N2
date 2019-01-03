#include "frameworks/time.h"

static const int8_t g_daysEachMonth[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
static inline uint8_t isLeap(int8_t year) {
  return (((year & 3) == 2)); //  && year != 30
}

static const int16_t g_divers[] = {60,60,24};
uint32_t time_toSecond(time_t* time) {
  uint16_t day;
  if (time->_day) {
    day = time->_day;
  } else {
    //TODO
    day = 0;
  	int16_t year = 0;
  	while(year < time->year){
  		int16_t days = isLeap(year)?366:365;
  		day += days;
  		year ++;
  	}
  	int16_t month = 0;
  	while(month < time->month){
  		day += g_daysEachMonth[month];
  		if(month == 1){
  			if(isLeap(time->year)){
  				day ++;
  			}
  		}
  		month ++;
  	}
  	day += time->day;
  }
  uint32_t second = 0;
  second += day * 24 * 60 * 60;
  second += time->hour * 60 * 60;
  second += time->minute * 60;
  second += time->second;
  return second - (TIME_ZONE*3600);
}
void time_fromSecond(uint32_t timestamp, time_t* time) {
  int8_t index = 0;
  timestamp += (TIME_ZONE*3600);
  uint32_t value = timestamp;
  while (index < 3) {
    int32_t d = value/g_divers[index];
    *(&time->second + index) = value - d*g_divers[index];
    index++;
    value = d;
  }
  int16_t day = value;
  int16_t year = 0;
  while ( 1 ) {
    int16_t days = isLeap(year)?366:365;
    if (day < days){
      break;
    }
    day -= days;
    year ++;
  }
  int16_t month = 0;
  while ( month < 12 ) {
    int16_t days = g_daysEachMonth[month];
    if (month == 1) {
      if (isLeap(year)) {
        days ++;
      }
    }
    if (day < days){
      break;
    }
    day -= days;
    month ++;
  }
  time->day = day;
  time->month = month;
  time->year = year;
  time->weekday = (value+4) % 7;
  time->_day = value;
}

static uint32_t g_timestamp;
static uint8_t g_hw_fetched;

//static uint32_t hw_fetch(void) {
//  if(!g_hw_fetched) {
//    g_hw_fetched = 1;
//    g_timestamp = time_hw_getTime();
//    if (g_timestamp) {
//      time_hw_reset();
//      return g_timestamp;
//    }
//  }
//  return 0;
//}

uint32_t time_getTime(void) {
//  if (g_timestamp == 0){
//    return hw_fetch();
//  }
  return g_timestamp;
}

void time_setTime(uint32_t timestamp) {
  g_timestamp = timestamp;
//  time_hw_reset();
//  time_hw_setTime(timestamp);
}

void time_init(void) {
  if (g_timestamp == 0) {
//    hw_fetch();
  }
}
