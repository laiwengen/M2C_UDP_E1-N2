#ifndef HTS221_H__
#define HTS221_H__

#include "stdint.h"

typedef struct hts221_t hts221_t;

typedef void(*hts221_temperatureFunction_t) (hts221_t*, int16_t);
typedef void(*hts221_humidityFunction_t) (hts221_t*, int16_t);

struct hts221_t{
  hts221_temperatureFunction_t temperatureFunction;
  hts221_humidityFunction_t humidityFunction;
  int16_t temperature;
  int16_t humidity;
};

void hts221_init(hts221_t* hts221);
void hts221_enable(hts221_t* hts221, uint8_t enable);

void hts221_hw_config(int8_t address, uint8_t value);
uint8_t hts221_hw_status(void);
void hts221_hw_start(void);
uint8_t hts221_hw_fetchData(int16_t* buffer);
uint8_t hts221_hw_fetchCalibrate(uint8_t* buffer);
void hts221_hw_init(void);
#endif
