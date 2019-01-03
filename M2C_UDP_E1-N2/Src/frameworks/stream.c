#include "stdlib.h"
#include "frameworks/stream.h"
#include "frameworks/tick.h"

//stream_t

list_t * g_stream_head;

static uint32_t getTick(void) {
  return tick_ms();
}

static void firePeekEvent(stream_t* stream, stream_listener_t* listener, int16_t index, int16_t size) {
  uint8_t* buffer = (uint8_t*)malloc(size);
  if (buffer){
    cb_peek(stream->circularBuffer, index, buffer, size);
    listener->eventFunction(stream, listener, buffer, size);
    free(buffer);
  } else {
    cb_drop(stream->circularBuffer, size);
  }
}


static void fireEvent(stream_t* stream, stream_listener_t* listener, int16_t size) {
  uint8_t* buffer = (uint8_t*)malloc(size);
  if (buffer){
    cb_read(stream->circularBuffer, buffer, size);
    listener->eventFunction(stream, listener, buffer, size);
    free(buffer);
  } else {
    cb_drop(stream->circularBuffer, size);
  }
}

static void checkListener(stream_t* stream, stream_listener_t* listener) {
	if (stream->circularBuffer == 0) {
		return;
	}
  switch (listener->type) {
    case STREAM_LISTENER_TYPE_peek: {
      int16_t size = cb_sizeAfter(stream->circularBuffer, listener->cache.index);
      if (size > 0) {
        firePeekEvent(stream, listener, listener->cache.index, size);
        cb_offsetIndex(stream->circularBuffer, &listener->cache.index, size);
      }
    }
    break;
    case STREAM_LISTENER_TYPE_length: {
      if (listener->data.length != 0 && cb_size(stream->circularBuffer) >= listener->data.length) {
        fireEvent(stream, listener, listener->data.length);
      }
    }
    break;
    case STREAM_LISTENER_TYPE_endByte: {
      int16_t size = cb_sizeAfter(stream->circularBuffer, listener->cache.index);
      for (int16_t i = 0; i < size; i++) {
				int16_t index = listener->cache.index;
        cb_offsetIndex(stream->circularBuffer, &listener->cache.index, 1);
        if (cb_peekIndex(stream->circularBuffer,index) == listener->data.endWith){
          fireEvent(stream, listener, cb_sizeBefore(stream->circularBuffer,listener->cache.index));
        }
      }
    }
    break;
    case STREAM_LISTENER_TYPE_tick: {
      int16_t size = cb_sizeAfter(stream->circularBuffer, listener->cache.tick.index);
      if (size > 0) {
        cb_offsetIndex(stream->circularBuffer, &listener->cache.tick.index, size);
        listener->cache.tick.tick = getTick();
      } else {
        uint32_t past = getTick() - listener->cache.tick.tick;
        if (past > listener->data.ticks) {
					int16_t cbs = cb_sizeBefore(stream->circularBuffer, listener->cache.tick.index);
					if (cbs>0){
						fireEvent(stream, listener, cbs);
					}
        }
      }
    }
    break;
  }
}
static void checkListeners(stream_t* stream) {
  list_t* l = stream -> listeners;
  while (l) {
    checkListener(stream,(stream_listener_t*)l->this.raw);
    l = l->next;
  }
}

void stream_run(void** params) {
  list_t* l = g_stream_head;
  while (l) {
    checkListeners((stream_t*)l->this.raw);
    l = l->next;
  }
}

list_t* stream_add(stream_t* stream) {
  return list_add0(&g_stream_head, stream);
}

list_t* stream_addListener( stream_t* stream, stream_listener_t* listener) {
  return list_add0(&stream->listeners, listener);
}

stream_listener_t* stream_removeListener( stream_t* stream, stream_listener_t* listener) {
  return (stream_listener_t*)list_remove(&stream->listeners, listener);
}

int16_t stream_flush(stream_t* stream) {
  if (stream->circularBuffer) {
    return stream->flushFunction(stream, 0);
    // if (cb_readable(stream->circularBuffer)) {
    //   int16_t size = cb_size(stream->circularBuffer);
    //   for (int16_t i = 0; i < size; i++) {
    //     uint8_t byte;
    //     cb_read(stream->circularBuffer, &byte, 1);
    //     stream->flushFunction(stream, byte);
    //   }
    //   return size;
    // } else {
    //   return stream->flushFunction(stream, 0);
    // }
  }
  return 0;
}

int16_t stream_write(stream_t* stream, const uint8_t* buffer, int16_t size) {
  int16_t writen = 0;
  if (stream->circularBuffer) {
    writen = cb_write(stream->circularBuffer, buffer, size);
  }
  if (size && !writen && stream->flushFunction) {
    for (int16_t i = 0; i < size; i++) {
      stream->flushFunction(stream, buffer[i]);
    }
    writen += size;
  }
  return writen;
}
