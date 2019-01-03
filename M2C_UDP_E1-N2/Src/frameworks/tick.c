#include "frameworks/tick.h"
#include "frameworks/thread.h"
#define TICK_THREAD_GROUP_ID 0x457543e6

volatile uint8_t g_tick_started = 0;
volatile uint32_t g_tick_tick = 0;

void tick_start(void) {
	g_tick_started = 1;
}

uint32_t tick_getTick(void) {
  return g_tick_tick;
}

void tick_stop(void) {
	g_tick_started = 0;
}

void tick_init(void) {
	tick_hw_init();
	thread_init(TICK_THREAD_GROUP_ID,tick_getTick);
}

void tick_add(thread_executeFunction_t function, uint32_t interval, void** params) {
	thread_quickAdd(TICK_THREAD_GROUP_ID, function, interval, params);
}

void tick_tick(void) {
	g_tick_tick ++;
	if (g_tick_started) {
		thread_run(TICK_THREAD_GROUP_ID);
	}
}
