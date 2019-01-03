#ifndef DUSTEX_H__
#define DUSTEX_H__
#include "stdint.h"

typedef struct dustEx_t dustEx_t;
typedef void (*dustEx_dataFunction_t)(dustEx_t*, uint8_t* buffer);
typedef void (*dustEx_errorFunction_t)(dustEx_t*);

struct dustEx_t {
  uint32_t id;
  dustEx_dataFunction_t dataFunction;
  dustEx_errorFunction_t errorFunction;
  uint32_t errorCount;
  uint8_t enabled;
};

void dustEx_init(dustEx_t* dustEx);
void dustEx_enable(dustEx_t* dustEx, uint8_t enable);

uint8_t dustEx_hw_receive(dustEx_t* dustEx, uint8_t* buffer, int16_t size);

uint8_t dustEx_hw_send(dustEx_t* dustEx, uint8_t* buffer, int16_t size);

void dustEx_hw_init(dustEx_t* dustEx);
void dustEx_hw_enable(dustEx_t* dustEx, uint8_t enable);

#endif
