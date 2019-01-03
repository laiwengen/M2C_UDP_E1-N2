#include "frameworks/sht3x.h"
#include "frameworks/schedule.h"

#define SCHEDULE_ID 0xd7173d37

static sht3x_t* g_sht3x;

static void sht3x_watcher(void** params) {
  uint8_t data[6];
  if (sht3x_hw_fetchData(data)) {
    {
      //TODO CRC
      int16_t value = 17500*((int32_t)(data[0]<<8 | data[1]))/0xffff - 4500;
      if (value != g_sht3x->temperature) {
        g_sht3x->temperature = value;
        g_sht3x->temperatureFunction(g_sht3x, value);
      }
    }
    {
      //TODO CRC
      int16_t value = 10000*((int32_t)(data[3]<<8 | data[4]))/0xffff;
      if (value != g_sht3x->humidity) {
        g_sht3x->humidity = value;
        g_sht3x->humidityFunction(g_sht3x, value);
      }
    }
  }
}

void sht3x_init(sht3x_t* sht3x) {
	g_sht3x = sht3x;
	sht3x_hw_init();
}

void sht3x_enable(sht3x_t* sht3x, uint8_t enable) {
  //TODO disable
  sht3x_hw_start();
  schedule_repeat(SCHEDULE_ID, sht3x_watcher, 100, 0);
}
