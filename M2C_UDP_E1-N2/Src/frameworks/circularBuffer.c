#include "frameworks/circularBuffer.h"
#include "frameworks/number.h"

static inline int16_t offset2Index(circularBuffer_t* cb, int16_t offset) {
  return offset % cb->size;
}

static inline uint8_t getIndexs(circularBuffer_t* cb, int16_t* readIndex, int16_t* writeIndex) {
  uint8_t flags = 0;
  if (cb->readIndex.raw & 0xffff0000) {
    *readIndex = cb->readIndex.function(cb);
    flags|=1;
  } else {
    *readIndex = cb->readIndex.value;
  }
  if (cb->writeIndex.raw & 0xffff0000) {
    *writeIndex = cb->writeIndex.function(cb);
    flags|=2;
  } else {
    *writeIndex = cb->writeIndex.value;
  }
	return flags;
}

int16_t cb_size(circularBuffer_t* cb) {
  int16_t writeIndex, readIndex;
  uint8_t flags = getIndexs(cb, &readIndex, &writeIndex);
  return offset2Index(cb, writeIndex + cb->size - readIndex);
}
int16_t cb_sizeAfter(circularBuffer_t* cb, int16_t index) {
  int16_t writeIndex, readIndex;
  uint8_t flags = getIndexs(cb, &readIndex, &writeIndex);
  return offset2Index(cb, writeIndex + cb->size - index);
}

int16_t cb_sizeBefore(circularBuffer_t* cb, int16_t index) {
  int16_t writeIndex, readIndex;
  uint8_t flags = getIndexs(cb, &readIndex, &writeIndex);
  return offset2Index(cb, index + cb->size - readIndex);
}

int16_t cb_drop(circularBuffer_t* cb, uint16_t size) {
  int16_t writeIndex, readIndex;
  uint8_t flags = getIndexs(cb, &readIndex, &writeIndex);
  if ((flags&1)) {
    return 0; // can'd read manuelly if read index is readonly;
  }
  int16_t dropSize = min(size, offset2Index(cb, writeIndex + cb->size - readIndex));
  cb -> readIndex.value = offset2Index(cb, readIndex + dropSize);
  return dropSize;
}
uint8_t cb_peekOne(circularBuffer_t* cb, int16_t offset) {
  int16_t writeIndex, readIndex;
  uint8_t flags = getIndexs(cb, &readIndex, &writeIndex); //TODO not need writeIndex
  return cb->buffer[offset2Index(cb, readIndex + offset)];
}
uint8_t cb_peekIndex(circularBuffer_t* cb, int16_t index) {
  return cb->buffer[offset2Index(cb, index)];
}
static int16_t peek(circularBuffer_t* cb, uint8_t* buffer, int16_t writeIndex, int16_t readIndex, uint16_t size) {
  int16_t readSize = 0;
  for (int16_t i = 0; i < size; i++) {
    int16_t ri = offset2Index(cb, readIndex + i);
    if (ri != writeIndex) {
      buffer[i] = cb->buffer[ri];
      readSize++;
    } else {
      break;
    }
  }
  return readSize;
}

int16_t cb_peek(circularBuffer_t* cb, uint16_t startIndex, uint8_t* buffer, uint16_t size) {
  int16_t writeIndex, readIndex;
  uint8_t flags = getIndexs(cb, &readIndex, &writeIndex);
  if ((flags&1)) {
    return 0; // can'd read manuelly if read index is readonly;
  }
  int16_t readSize = peek(cb, buffer, writeIndex, startIndex, size);
  return readSize;
}

int16_t cb_read(circularBuffer_t* cb, uint8_t* buffer, uint16_t size) {
  int16_t writeIndex, readIndex;
  uint8_t flags = getIndexs(cb, &readIndex, &writeIndex);
  if ((flags&1)) {
    return 0; // can'd read manuelly if read index is readonly;
  }
  int16_t readSize = peek(cb, buffer, writeIndex, readIndex, size);
  cb -> readIndex.value = offset2Index(cb, readIndex + readSize);
  return readSize;
}

int16_t cb_write(circularBuffer_t* cb, const uint8_t* buffer, uint16_t size) {
  int16_t writeIndex, readIndex;
  uint8_t flags = getIndexs(cb, &readIndex, &writeIndex);
  if ((flags&2)) {
    return 0; // can'd write manuelly if read index is readonly;
  }
  int16_t writeSize = 0, overflow = 0;
  for (int16_t i = 0; i < size; i++) {
    int16_t wi = offset2Index(cb, writeIndex + i);
    int16_t nwi = offset2Index(cb, writeIndex + i + 1);
    if (nwi != readIndex) { // not full
      cb->buffer[wi] = buffer[i];
      writeSize++;
    } else if (flags & 1) { // overflow if readIndex is not readonly
      cb->buffer[wi] = buffer[i];
      writeSize ++;
      overflow++;
    } else {
      break;
    }
  }
  if ((flags & 1) == 0 && overflow > 0) {
    cb -> readIndex.value = offset2Index(cb, readIndex + overflow);
  }
  cb -> writeIndex.value = offset2Index(cb, writeIndex + writeSize);
  return writeSize;
}

void cb_offsetIndex(circularBuffer_t* cb, int16_t* index, int16_t offset){
  *index = offset2Index(cb, *index + offset);
}

uint8_t cb_readable(circularBuffer_t* cb) {
  int16_t writeIndex, readIndex;
  uint8_t flags = getIndexs(cb, &readIndex, &writeIndex);
  return ((flags&1) == 0);
}

uint8_t cb_writable(circularBuffer_t* cb) {
  int16_t writeIndex, readIndex;
  uint8_t flags = getIndexs(cb, &readIndex, &writeIndex);
  return ((flags&2) == 0);
}
