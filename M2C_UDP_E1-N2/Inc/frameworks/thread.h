#ifndef THREAD_H__
#define THREAD_H__
#include "stdint.h"
#include "frameworks/list.h"

typedef void (*thread_executeFunction_t) (void** params);
typedef uint32_t (*thread_fetchTickFunction_t) (void);
typedef struct thread_t {
	uint32_t id;
	uint32_t remainTimes;
	uint32_t intervalTick;
	uint32_t executeTick;
	thread_executeFunction_t function;
	void** params;
} thread_t;

typedef struct threadGroup_t {
	uint32_t id;
	list_t* threads;
	thread_fetchTickFunction_t fetchTickFunction;
	uint32_t idGenerator;
} threadGroup_t;


threadGroup_t* thread_init(uint32_t groupId, thread_fetchTickFunction_t function);
void thread_deinit(uint32_t groupId);
thread_t* thread_exists(uint32_t groupId, uint32_t tid);
uint8_t thread_add(uint32_t groupId, thread_t* t, uint32_t followId);
uint32_t thread_quickAdd(uint32_t groupId, thread_executeFunction_t function, uint32_t interval, void** params);
uint8_t thread_remove(uint32_t groupId, uint32_t tid);
void thread_execute(uint32_t groupId, uint32_t tid);
void thread_run(uint32_t groupId);


#endif
