#ifndef AVERAGE_H__
#define AVERAGE_H__
#include "stdint.h"

typedef struct {
	uint8_t* buffer;
	uint16_t size;
	uint8_t bpn;
	uint8_t shift;
	uint8_t index;
	uint8_t length;
} average_sma_t;

typedef struct {
	uint64_t sum;
	uint32_t cnt;
} average_cma_t;

typedef struct {
	uint16_t percent;
	uint32_t current;
	uint32_t all;
	uint32_t matched;
} average_pma_t;

typedef struct {
	uint16_t const* x;
	uint16_t const* y;
	uint8_t size;
	uint8_t shiftX;
	uint8_t shiftY;
} average_tab_t;

void average_sma_push(average_sma_t* sma, uint32_t value);
uint32_t average_sma_get(average_sma_t* sma, uint16_t index);
uint32_t average_sma_average(average_sma_t* sma, uint16_t startInclude, uint16_t endExclude, uint16_t mul);
uint64_t average_sma_noise(average_sma_t* sma);

void average_cma_push(average_cma_t* cma, uint32_t sum, uint32_t weight);
uint32_t average_cma_peek(average_cma_t* cma, uint32_t mul);
uint32_t average_cma_get(average_cma_t* cma, uint32_t mul);

void average_pma_push(average_pma_t* pma, uint32_t all, uint32_t matched);
void average_pma_correct(average_pma_t* pma, int8_t feedback);

uint32_t average_tab_correct(average_tab_t const* tab, uint32_t input);

int32_t average_ema_shift(int32_t source, int32_t value, int8_t shift);
#endif
