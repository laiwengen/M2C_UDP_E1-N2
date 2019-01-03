#ifndef STEAM_H__
#define STEAM_H__
#include "stdint.h"
#include "frameworks/circularBuffer.h"
#include "frameworks/list.h"

struct stream_t;
struct stream_listener_t;

typedef void (*stream_eventFunction_t) (struct stream_t* stream, struct stream_listener_t* listener, uint8_t* buffer, int16_t size);

typedef int16_t (*stream_flushFunction_t) (struct stream_t* stream, uint8_t byte);

enum stream_listenerType_t{
  STREAM_LISTENER_TYPE_peek,
  STREAM_LISTENER_TYPE_length,
  STREAM_LISTENER_TYPE_tick,
  STREAM_LISTENER_TYPE_endByte,
  STREAM_LISTENER_TYPE_ignore,
  STREAM_LISTENER_TYPE_SIZE,
};

typedef struct stream_listener_t{
  uint8_t type;
  stream_eventFunction_t eventFunction;
  union {
    uint8_t endWith;
    int16_t length;
    int16_t ticks;
  } data;
  union {
    int16_t index;
    struct {
      int16_t index;
      uint32_t tick;
    } tick;
  } cache;
} stream_listener_t;

typedef struct stream_t{
  circularBuffer_t* circularBuffer;
  list_t* listeners;
  stream_flushFunction_t flushFunction;
} stream_t;


void stream_run(void** params);
list_t* stream_add(stream_t* stream);
list_t* stream_addListener( stream_t* stream, stream_listener_t* listener);
stream_listener_t* stream_removeListener( stream_t* stream, stream_listener_t* listener);
int16_t stream_flush(stream_t* stream);
int16_t stream_write(stream_t* stream, const uint8_t* buffer, int16_t size);

#endif
