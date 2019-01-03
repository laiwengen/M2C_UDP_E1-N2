#ifndef BATTERY_H__
#define BATTERY_H__

#include "stdint.h"

typedef struct battery_t battery_t;
typedef void(*battery_percentFunction_t) (battery_t*, int16_t);
typedef void(*battery_chargingFunction_t) (battery_t*, uint8_t);
struct battery_t {
  int16_t percent;
  uint8_t charging;
  battery_percentFunction_t percentFunction;
  battery_chargingFunction_t chargingFunction;
};

void battery_watcher(void** params);
void battery_init(battery_t* battery);

int16_t battery_hw_getPercent(void);
uint8_t battery_hw_isCharging(void);
uint8_t battery_charging(void);
void battery_hw_init(void);

#endif
