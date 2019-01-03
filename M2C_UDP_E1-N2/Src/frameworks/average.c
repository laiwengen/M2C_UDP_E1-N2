#include "math.h"
#include "frameworks/number.h"
#include "frameworks/average.h"

static uint32_t getBufferValue(uint8_t* buffer, uint8_t bpn) {
  return (uint32_t)number_valueInt32(buffer, bpn);
}
static uint32_t smaRawGet(average_sma_t* sma, uint16_t index, uint16_t size) {
	int16_t offset = (sma->index - index - 1);
	while (offset < 0) {
		offset += size;
	}
	uint8_t* buffer = sma->buffer + offset * sma->bpn;
	return getBufferValue(buffer, sma->bpn);
}
void average_sma_push(average_sma_t* sma, uint32_t value) { // not suport negetive value
	if (sma->shift > 0) {
		value = (value + (1 << (sma->shift - 1))) >> sma->shift;
	}
	if (value >= (UINT32_C(1) << (8 * sma->bpn))) {
		value = UINT32_MAX;
	}
	uint8_t* p = sma->buffer + sma->index * sma->bpn;
	for (uint8_t i = 0; i < sma->bpn; i++) {
		*(p + i) = value >> (i << 3);
	}
	uint16_t size = sma->size / sma->bpn;
	sma->index = sma->index + 1;
	if (sma->index >= size) {
		sma->index = 0;
	}
	if (sma->length < size) {
		sma->length++;
	}
}

uint32_t average_sma_average(average_sma_t* sma, uint16_t startInclude, uint16_t endExclude, uint16_t mul){
	uint32_t sum = 0;
	int16_t cnt = 0;
	uint16_t size = sma->size / sma->bpn;
	for (int16_t i = startInclude; i < sma->length && i < endExclude; i++) {
		sum += smaRawGet(sma, i, size);
		cnt++;
	}
	if (cnt == 0) {
		return 0;
	}
	//if (sum > (INT64_MAX >> sma->shift) || -sum > (INT64_MAX >> sma->shift)) {
	//	return (sum + (cnt >> 1)) / cnt * (1 << sma->shift);
	//} else {
	//}
	if (mul == cnt) {
		return sum * (1 << sma->shift);
	}
	return (sum * (1 << sma->shift) * mul + (cnt >> 1)) / cnt;
}

uint32_t average_sma_get(average_sma_t* sma, uint16_t index) {
	if (index > sma->length) {
		return 0;
	}
	uint16_t size = sma->size / sma->bpn;
	return smaRawGet(sma, index, size) << sma->shift;
}


uint64_t average_sma_noise(average_sma_t* sma) {
	if (sma->length <= 2) {
		return 0;
	}
	uint64_t sum = 0;
	uint16_t cnt = 0;
	uint16_t size = sma->size / sma->bpn;
	uint32_t left = smaRawGet(sma, 0, size);
	uint32_t center = smaRawGet(sma, 1, size);
	for (uint16_t i = 2; i < sma->length; i++) {
		uint32_t right = smaRawGet(sma, i, size);
		int32_t r2 = smaRawGet(sma, i, size) + smaRawGet(sma, i - 2, size) - 2 * smaRawGet(sma, i - 1, size);
		sum += (r2 * (int16_t)r2 + 2) / 4;
		cnt++;
		left = center;
		center = right;
	}
	//for (int i = 2; i < size; i++) {
	//	int32_t r2 = smaRawGet(sma,i,size) + smaRawGet(sma,i - 2,size) - 2 * smaRawGet(sma,i - 1,size);
	//	sum += (r2 * (int16_t)r2 + 2) / 4;
	//	cnt ++;
	//}
	return ((sum << (sma->shift * 2)) + (cnt >> 1)) / cnt;
}

void average_cma_push(average_cma_t* cma, uint32_t sum, uint32_t weight) {
	cma->sum += sum * weight;
	cma->cnt += weight;
}
uint32_t average_cma_peek(average_cma_t* cma, uint32_t mul) {
	if (cma->cnt == 0) {
		return 0;
	}
	return (cma->sum * mul + (cma->cnt >> 2)) / cma->cnt;
}
uint32_t average_cma_get(average_cma_t* cma, uint32_t mul) {
	uint32_t toRet = average_cma_peek(cma, mul);
	cma->sum = 0;
	cma->cnt = 0;
	return toRet;
}

void average_pma_push(average_pma_t* pma, uint32_t all, uint32_t matched) {
	pma->all += all;
	pma->matched += matched;
}
void average_pma_correct(average_pma_t* pma, int8_t feedback) {
	if (pma->all == 0) {
		return;
	}
	uint32_t expect = pma->matched << 12;
	uint32_t all = (pma->all * pma->percent);
	uint32_t diver;
	uint32_t suber;
	if (expect > all) {
		diver = expect;
		suber = expect - all;
	} else {
		diver = all;
		suber = all - expect;
	}
	suber = (((pma->current + 0x1000) * (uint64_t)suber * suber) >> (16 - feedback)) / (diver * (uint64_t)diver);
	if (expect > all) {
		if (suber > pma->current) {
			pma->current <<= 1;
		} else {
			pma->current += suber;
		}
	} else {
		if (suber * 2 > pma->current) {
			pma->current >>= 1;
		} else {
			pma->current -= suber;
		}
	}
	pma->all = 0;
	pma->matched = 0;
}

#define TABLE_K_SHIFT 24
uint32_t average_tab_correct(average_tab_t const* tab, uint32_t input) {
	uint16_t const* x = tab->x;
	uint16_t const* y = tab->y;
	uint8_t shiftX = tab->shiftX;
	uint8_t shiftY = tab->shiftY;
	uint8_t size = tab->size;
	uint32_t xs = 0,ys = 0;
	uint64_t k;
	uint16_t last1 = 0xff,last2 = 0xff;
	uint16_t i;
	uint64_t dummyValue = UINT64_C(1) << (TABLE_K_SHIFT - shiftX + shiftY);
	for (i = 0; i < size; i++)
	{
		//if (y[i] != 0)
		{
			int32_t tx = x[i]<<shiftX;
			if (input< tx)
			{
				break;
			}
			last2 = last1;
			last1 = i;
		}
	}
	if (last1 == 0xff)
	{
		if(i==size)
		{
			return UINT32_MAX;
		}
		else
		{
			k = (y[i]*dummyValue + (x[i]>>1))/x[i];
		}
	}
	else
	{
		xs = x[last1]<<shiftX;
		ys = y[last1]<<shiftY;
		if(i==size)
		{
			if(last2 == 0xff)
			{
				k = (y[last1]*dummyValue + (x[last1]>>1))/x[last1];
			}
			else
			{
				k = ((y[last1] - y[last2])*dummyValue + ((x[last1]-x[last2])>>1))/(x[last1]-x[last2]);
			}
		}
		else
		{
			k = ((y[i] - y[last1])*dummyValue + ((x[i]-x[last1])>>1))/(x[i]-x[last1]);
		}
	}
	return (uint32_t) (ys + (((input - xs)*(k) + (1 << (TABLE_K_SHIFT-1))) >> TABLE_K_SHIFT));
}

int32_t average_ema_shift(int32_t source, int32_t value, int8_t shift) {
  int32_t delta = value - source;
  if (delta == 0) {
	} else if (delta > 0) {
    delta = (delta >> shift) + 1;
  } else {
    delta = -((-delta) >> shift) - 1;
  }
  return source + delta;
}
