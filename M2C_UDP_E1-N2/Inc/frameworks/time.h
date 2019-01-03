#ifndef TIME_H__
#define TIME_H__
#include "stdint.h"

#define TIME_ZONE (8)
#define TIME_ZONE_OFFSET_IN_SECOND (TIME_ZONE * 3600)

static inline uint32_t time_aligned(uint32_t timestamp, uint32_t base) {
  return ((timestamp + TIME_ZONE_OFFSET_IN_SECOND) / base * base) - TIME_ZONE_OFFSET_IN_SECOND;
}

typedef struct time_t {
  uint8_t second;
  uint8_t minute;
  uint8_t hour;
  uint8_t day;
  uint8_t month;
  uint8_t year;
  uint8_t weekday;
  uint16_t _day;
} time_t;

uint32_t time_hw_past(void);
void time_hw_reset(void);
void time_hw_setTime(uint32_t timestamp);
uint32_t time_hw_getTime(void);

uint32_t time_toSecond(time_t* time);
void time_fromSecond(uint32_t timestamp, time_t* time);
void time_setTime(uint32_t timestamp);
uint32_t time_getTime(void);

#endif
