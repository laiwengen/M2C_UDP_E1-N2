#include "frameworks/hts221.h"
#include "frameworks/schedule.h"

#define HTS221_SCHEDULE_ID 0x07552a9b

static hts221_t* g_hts221;
static int16_t g_calbrateData[8]; // x0,x1, y0,y1

static int16_t a2v(int16_t*cal, int32_t a, int16_t multi) {
  int16_t d = (cal[1]-cal[0]);
	if (d == 0) {
		return 0;
	}
  return ((a-cal[0]) * (cal[3] - cal[2]) * multi + d/2) / d + cal[2]*multi;
}

static void hts221_watcher(void** params) {
  // hts221_hw_start();
	uint32_t status = hts221_hw_status();
  if ((status & 0x03) == 0x03) {
		int16_t data[2];
		if (hts221_hw_fetchData(data)) {
			if ((status & 0x01) != 0) { // temperature
				int16_t value = a2v(g_calbrateData,data[1],25)/2;
				if (value != g_hts221->temperature) {
					g_hts221->temperature = value;
					g_hts221->temperatureFunction(g_hts221, value);
				}
			}
			if ((status & 0x02) != 0) { // humidity
				int16_t value = a2v(g_calbrateData+4,data[0],50);
				if (value != g_hts221->humidity) {
					g_hts221->humidity = value;
					g_hts221->humidityFunction(g_hts221, value);
				}
			}
		}
  }
}
static void hts221_cmdInit(void** params) {
	hts221_hw_config(0x20, 0x85);
  uint8_t buffer[16];
  if (hts221_hw_fetchCalibrate(buffer)) {
    g_calbrateData[0] = *(int16_t*)(buffer+0xc);
    g_calbrateData[1] = *(int16_t*)(buffer+0xe);
    g_calbrateData[2] = buffer[2]|((buffer[5]&0x03)<<8);
    g_calbrateData[3] = buffer[3]|((buffer[5]&0x0c)<<6);
    g_calbrateData[4] = *(int16_t*)(buffer+0x6);
    g_calbrateData[5] = *(int16_t*)(buffer+0xa);
    g_calbrateData[6] = buffer[0];
    g_calbrateData[7] = buffer[1];
  }
}

void hts221_init(hts221_t* hts221) {
	g_hts221 = hts221;
	hts221_hw_init();
}

void hts221_enable(hts221_t* hts221, uint8_t enable) {
  schedule_once(0, hts221_cmdInit, 50, 0);
  schedule_repeat(HTS221_SCHEDULE_ID, hts221_watcher, 100, 0);
}
