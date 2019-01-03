#include "string.h"
#include "frameworks/number.h"
#include "frameworks/list.h"
#include "frameworks/history.h"

#define history_length(h) ((h)->size/(h)->bpd)


int16_t history_offsetToIndex(history_t* history, int32_t offset) {
  int32_t length = history_length(history);
  if (offset >=0 || offset < - length) {
    return -1;
  }
  int16_t index = offset + history->index + length;
  if (index >= length) {
    index -= length;
  }
  return index;
}

int32_t history_getOffset(history_t* history, uint32_t time) {
  return ((int32_t)(time - history->endTime))/(int32_t)history->intervalTime;
}

uint8_t history_getData(history_t* history, uint32_t time, void* out) {
  int32_t offset =history_getOffset(history, time);
  int16_t index = history_offsetToIndex(history, offset);
  if (index >= 0) {
    memcpy(out, history->buffer + index * history->bpd, history->bpd);
    return 1;
  }
  return 0;
}

uint8_t history_setData(history_t* history, uint32_t time, void* data) {
  int32_t offset =history_getOffset(history, time);
  int16_t index = history_offsetToIndex(history, offset);
  if (index >= 0) {
    memcpy(history->buffer + index * history->bpd, data, history->bpd);
    return 1;
  }
  return 0;
}

static inline void incIndex(int16_t* index, int32_t length) {
  int16_t i = (*index) + 1;
  if (i >= length) {
    i -= length;
  }
  (*index) = i;
}

static inline void setData(history_t* history, int16_t index, void* data) {
  memcpy(history->buffer + index * history->bpd, data, history->bpd);
}

uint8_t history_addData(history_t* history, uint32_t time, void* data) {
  int32_t offset = history_getOffset(history, time);
  int32_t length = history_length(history);
  uint8_t empty[8] = {0};
  if (data == 0) {
    data = empty;
  }
  if (offset < 0) {
    return 0;
  } else if (offset >= length - 1) {
    // int16_t index = 0;
    fors(length-1) {
      setData(history, i, empty);
    }
    setData(history, length-1, data);
    history->index = 0;
    history->endTime = time + history->intervalTime;
  } else {
    fors (offset) {
      setData(history, history->index, empty);
      incIndex(&history->index, length);
    }
    setData(history, history->index, data);
    incIndex(&history->index, length);
    history->endTime += (offset + 1) * history->intervalTime;
  }
  return 1;
}

uint8_t history_setEndTime(history_t* history, uint32_t endTime) {
  if (history_addData(history, endTime - history->intervalTime, 0)) {
    history->endTime = endTime;
    return 1;
  }
  return 0;
}

void history_reset(history_t* history) {
  history->endTime = 0;
  history->index = 0;
  memset(history->buffer, 0, history->size);
}

// uint32_t history_pushData(history_t* history, void* data) {
//   int16_t index;
//   uint32_t timestamp = history->endTime;
//   if (history->length * history->bpd < history->size) {
//     index = history->length;
//     history->length ++;
//   } else {
//     index = history->index;
//     offset = history->length;
//     history->index ++;
//     history->endTime += history->intervalTime;
//     if (history->index >= history->length) {
//       history->index -= history->length;
//     }
//   }
//   memcpy(history->buffer + index * history->bpd, data, history->bpd);
//   return timestamp;
// }
uint8_t history_getMaxMinData(history_t* history, uint32_t startTime, uint32_t endTime, int32_t* max, int32_t* min) {
  int32_t startOffset = history_getOffset(history, startTime), endOffset = history_getOffset(history, endTime);
  int32_t length = history_length(history);
  if (startOffset < -length) {
    startOffset = 0;
  }
  if (endOffset > 0) {
    endOffset = 0;
  }
  if (startOffset >= endOffset) {
    return 0;
  }
  int32_t maxValue = INT32_MIN;
  int32_t minValue = INT32_MAX;
  forsei(startOffset, endOffset, 1) {
    int16_t index = history_offsetToIndex(history, i);
    if (index >= 0){
      int32_t value = number_valueInt32(&history->buffer[index * history->bpd], history->bpd);
      maxValue = max(maxValue, value);
      minValue = min(minValue, value);
    }
  }
  *max = maxValue;
  *min = minValue;
  return 1;
}
int16_t history_forEach(history_t* history, uint32_t startTime, uint32_t endTime, history_function_t function, void** params) {
  int32_t startOffset = history_getOffset(history, startTime), endOffset = history_getOffset(history, endTime);
  int32_t length = history_length(history);
  if (startOffset < -length) {
    startOffset = -length;
  }
  if (endOffset > 0) {
    endOffset = 0;
  }
  if (startOffset >= endOffset) {
    return 0;
  }
  int16_t count = 0;
  forsei(startOffset, endOffset, 1) {
    int16_t index = history_offsetToIndex(history, i);
    if (index > 0){
      int32_t value = number_valueInt32(&history->buffer[index * history->bpd], history->bpd);
      if (function(history, i, value, params)){
        count++;
      }
    }
  }
  return count;
}
