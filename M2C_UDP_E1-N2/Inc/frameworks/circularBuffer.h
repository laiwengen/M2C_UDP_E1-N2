#ifndef CIRCULAR_BUFFER_H__
#define CIRCULAR_BUFFER_H__
#include "stdint.h"

struct circularBuffer_t;
typedef struct circularBuffer_t circularBuffer_t;

typedef int16_t(*cb_indexFunction_t)(circularBuffer_t*);

typedef union {
  int16_t value;
  cb_indexFunction_t function;
  uint32_t raw;
} cb_index_t;

typedef struct circularBuffer_t{
  uint8_t* buffer;
  int16_t size;
  cb_index_t readIndex;
  cb_index_t writeIndex;
} circularBuffer_t;

int16_t cb_size(circularBuffer_t* cb);
int16_t cb_sizeAfter(circularBuffer_t* cb, int16_t index);
int16_t cb_sizeBefore(circularBuffer_t* cb, int16_t index);
int16_t cb_drop(circularBuffer_t* cb, uint16_t size);
uint8_t cb_peekOne(circularBuffer_t* cb, int16_t offset);
uint8_t cb_peekIndex(circularBuffer_t* cb, int16_t index);

uint8_t cb_readable(circularBuffer_t* cb);
uint8_t cb_writable(circularBuffer_t* cb);

int16_t cb_peek(circularBuffer_t* cb, uint16_t startIndex, uint8_t* buffer, uint16_t size);
int16_t cb_read(circularBuffer_t* cb, uint8_t* buffer, uint16_t size);
int16_t cb_write(circularBuffer_t* cb, const uint8_t* buffer, uint16_t size);

void cb_offsetIndex(circularBuffer_t* cb, int16_t* index, int16_t offset);

#endif
