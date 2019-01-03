#include "stdlib.h"
#include "frameworks/schedule.h"
#include "frameworks/tick.h"

#define SCHEDULE_THREAD_GROUP_ID 0x6454535c

void schedule_run(void** params) {
	thread_run(SCHEDULE_THREAD_GROUP_ID);
}

uint32_t schedule_getTick(void) {
	return tick_ms();
}

void schedule_init() {
	thread_init(SCHEDULE_THREAD_GROUP_ID, schedule_getTick);
}

uint32_t addSchedule(uint32_t id, thread_executeFunction_t function, uint32_t tick, void** params, uint32_t remainTimes) {
	thread_remove(SCHEDULE_THREAD_GROUP_ID, id);
	thread_t* t = (thread_t*) malloc(sizeof(thread_t));
	if (t) {
		t->id = id;
		t->remainTimes = remainTimes;
		t->intervalTick = tick;
		t->executeTick = schedule_getTick() + tick;
		t->function = function;
		t->params = params;
		if (thread_add(SCHEDULE_THREAD_GROUP_ID,t,0)) {
			return t->id;
		} else {
			free(t); // check if address is in heap
		}
	}
	return 0;
}

uint32_t schedule_once(uint32_t id, thread_executeFunction_t function, uint32_t tick, void** params) {
	return addSchedule(id, function, tick, params, 1);
}

uint32_t schedule_multi(uint32_t id, thread_executeFunction_t function, uint32_t tick, void** params, uint32_t times) {
	return addSchedule(id, function, tick, params, times);
}

uint32_t schedule_repeat(uint32_t id, thread_executeFunction_t function, uint32_t tick, void** params) {
	return addSchedule(id, function, tick, params, 0);
}

void schedule_cancel(uint32_t id) {
	thread_remove(SCHEDULE_THREAD_GROUP_ID, id);
}

uint8_t schedule_exists(uint32_t id) {
	thread_t* t = thread_exists(SCHEDULE_THREAD_GROUP_ID, id);
	return t != 0;
}
void schedule_reset(uint32_t id, uint32_t tick) {
	thread_t* t = thread_exists(SCHEDULE_THREAD_GROUP_ID, id);
	t->executeTick = schedule_getTick() + tick;
}
