#ifndef SCHEDULE_H__
#define SCHEDULE_H__

#include "stdint.h"
#include "frameworks/thread.h"

uint32_t schedule_once(uint32_t id, thread_executeFunction_t function, uint32_t delayTick, void** params);
uint32_t schedule_multi(uint32_t id, thread_executeFunction_t function, uint32_t delayTick, void** params, uint32_t times);
uint32_t schedule_repeat(uint32_t id, thread_executeFunction_t function, uint32_t delayTick, void** params);
void schedule_cancel(uint32_t id);
uint8_t schedule_exists(uint32_t id);
void schedule_reset(uint32_t id, uint32_t tick);
void schedule_run(void**);
void schedule_tick(void**);
void schedule_init(void);


#endif
