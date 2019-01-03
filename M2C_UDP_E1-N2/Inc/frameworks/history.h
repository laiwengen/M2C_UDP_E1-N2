#ifndef HISTORY_H__
#define HISTORY_H__

#include "stdint.h"

typedef struct history_t {
  uint32_t endTime;
  int16_t index;
  int16_t size;
  uint8_t* buffer;
  uint32_t intervalTime;
  int8_t bpd; //byte per data;
} history_t;

typedef uint8_t (*history_function_t)(history_t*, int16_t, int32_t, void**);

int16_t history_offsetToIndex(history_t* history, int32_t offset);
int32_t history_getOffset(history_t* history, uint32_t time);
uint8_t history_getData(history_t* history, uint32_t time, void* out);
uint8_t history_getMaxMinData(history_t* history, uint32_t startTime, uint32_t endTime, int32_t* max, int32_t* min);
int16_t history_forEach(history_t* history, uint32_t startTime, uint32_t endTime, history_function_t function, void** params);

uint8_t history_setData(history_t* history, uint32_t time, void* data);
uint8_t history_setEndTime(history_t* history, uint32_t endTime);
uint8_t history_addData(history_t* history, uint32_t time, void* data);

void history_reset(history_t* history);
#endif
