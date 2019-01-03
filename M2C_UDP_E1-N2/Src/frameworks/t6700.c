#include "frameworks/t6700.h"
#include "frameworks/schedule.h"

#define T6700_SCHEDULE_ID 0x3353e2cc

static t6700_t* g_t6700;

static void t6700_watcher(void** params) { //TODO start poll every 4.8 s?
	uint16_t status = t6700_hw_status();
	if ((status & 0x0800) == 0) {
		int16_t value = t6700_hw_gasPpm();
		if (value>0 && value!= g_t6700->value) {
			g_t6700->value = value;
			g_t6700->dataFunction(g_t6700, value);
		}
	}
	uint8_t calibrating = (status & 0x8000) == 0;
	if (calibrating != g_t6700->calibrating) {
		g_t6700->calibrating = calibrating;
		g_t6700->calibratingFunction(g_t6700, calibrating);
	}
}
static void t6700_cmdInit(void** params) {
	t6700_hw_abcLogic(0);
}

uint8_t t6700_calibrating(void) {
  return (t6700_hw_status() & 0x8000)?1:0;
}
void t6700_calibrate(uint8_t enable) {
	t6700_hw_calibrate(enable);
	// g_t6700->calibrating = enable;
}

void t6700_init(t6700_t* t6700) {
	t6700_hw_init();
	g_t6700 = t6700;
}
void t6700_enable(t6700_t* t6700, uint8_t enable) {
	if (enable) {
	  schedule_once(0, t6700_cmdInit, 500, 0);
	  schedule_repeat(T6700_SCHEDULE_ID, t6700_watcher, 1000, 0);
		t6700_hw_enable(1);
	} else {
		t6700_hw_enable(0);
	}
}
