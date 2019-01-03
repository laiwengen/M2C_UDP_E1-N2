#include "frameworks/battery.h"
#include "frameworks/ad.h"
#include "frameworks/schedule.h"

#define BATTERY_SCHEDULE_ID 0xb0a51465

static battery_t* g_battery;

void battery_watcher(void** params) {
  uint8_t charging = battery_hw_isCharging();
  if (charging != g_battery->charging) {
    g_battery->charging = charging;
    g_battery->chargingFunction(g_battery, charging);
  }
  int16_t percent = battery_hw_getPercent();
  if (percent >= 0) {
    if (percent != g_battery->percent) {
      g_battery->percent = percent;
      g_battery->percentFunction(g_battery, percent);
    }
  }
}

uint8_t battery_charging(void) {
  return battery_hw_isCharging();
  // return 1;
}

void battery_init(battery_t* battery) {
  g_battery = battery;
	ad_init();
	battery_hw_init();
  schedule_repeat(BATTERY_SCHEDULE_ID, battery_watcher, 100, 0);
}
