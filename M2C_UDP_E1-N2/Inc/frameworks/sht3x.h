#ifndef SHT3X_H__
#define SHT3X_H__
#include "stdint.h"

typedef struct sht3x_t sht3x_t;
typedef void(*sht3x_dataFunction_t)(sht3x_t*, int32_t);

struct sht3x_t {
  sht3x_dataFunction_t temperatureFunction;
  sht3x_dataFunction_t humidityFunction;
  int16_t temperature;
  int16_t humidity;
};

void sht3x_init(sht3x_t* hts221);
void sht3x_enable(sht3x_t* hts221, uint8_t enable);

void sht3x_hw_start(void);
uint8_t sht3x_hw_fetchData(uint8_t* buffer);
void sht3x_hw_init(void);

#endif
