#ifndef T6700_H__
#define T6700_H__

#include "stdint.h"

typedef struct t6700_t t6700_t;

typedef void(*t6700_dataFunction_t) (t6700_t*, int16_t);
typedef void(*t6700_calibratingFunction_t) (t6700_t*, uint8_t);

struct t6700_t{
  t6700_dataFunction_t dataFunction;
  t6700_calibratingFunction_t calibratingFunction;
  int16_t value;
  uint8_t calibrating;
};

uint8_t t6700_calibrating(void);
void t6700_calibrate(uint8_t enable);
void t6700_init(t6700_t* t6700);
void t6700_enable(t6700_t* t6700, uint8_t enable);

int16_t t6700_hw_gasPpm(void);
uint16_t t6700_hw_status(void);
uint8_t t6700_hw_abcLogic(uint8_t enable);
uint8_t t6700_hw_calibrate(uint8_t enable);
void t6700_hw_enable(uint8_t enable);
void t6700_hw_init(void);
#endif
