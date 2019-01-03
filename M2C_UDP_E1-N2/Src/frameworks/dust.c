#include "string.h"
#include "math.h"
#include "frameworks/list.h"
#include "frameworks/number.h"
#include "frameworks/tick.h"
#include "frameworks/average.h"
#include "frameworks/schedule.h"
#include "frameworks/motor.h"
#include "frameworks/dust.h"

#define DATA_SHIFT 4
#define ACCUMULATE_COUNT_TO_CALCULATE 100
#define ACCUMULATE_CHANNELS 8
// #define DATA_MUL 20
#define DATA_CALCULATE_SIZE 16
#define DATA_OUTPUT_SIZE 9
#define DATA_PM2D5_INDEX 1
#define DATA_DIV_1 60097
#define DATA_DIV_3 50535
#define PMA_SUM_LIMIT 200
#define PMA_MATCH_LIMIT 10

//const uint32_t THRESHOLDS[5] = { 1600000, 10000000, 40000000, 160000000};
//const uint32_t THRESHOLDY[5] = {   92000,   400000,  1200000,   3660000,   16000000};
uint32_t (*g_dust_correctThresholdFunction) (uint32_t) = 0;
uint8_t g_dust_sensitivity = 5;
uint8_t g_dust_triggerLevel = 2;

#if RAW_DUST

#define RAW_SIZE 200
uint8_t g_dust_buffer[RAW_SIZE] = {0};
uint16_t g_dust_buffer_cnt;
uint16_t g_dust_calculate_cnt;

#else

#endif

enum {
	WAVE_LOW = 0,
	WAVE_HIGH,
	WAVE_RISE,
	WAVE_FALL,
};

//static inline int32_t pzpe(int32_t inpute) {
//	int32_t lx=0, ly = 0;
//	for (uint8_t i = 0; i<5; i++) {
//		if ( inpute < THRESHOLDS[i]){
//			int32_t rx = THRESHOLDS[i], ry = THRESHOLDY[i];
//			return (inpute - lx) * (uint64_t)(ry - ly) / (rx - lx);
//		}
//		lx = THRESHOLDS[i];
//		ly = THRESHOLDY[i];
//	}
//	return THRESHOLDY[4];
//}

// static void j2ci(uint8_t j){
// 	for (uint8_t i = 1; i<sizeof(THRESHOLD_INDEXS); i++){
// 		if (j<THRESHOLD_INDEXS[i]) {
// 			return i-1;
// 		}
// 	}
// 	return sizeof(THRESHOLD_INDEXS) - 1;
// }

const uint16_t EXP_EIGHTS[] = {32768, 35734, 38968, 42495, 46341, 50535, 55109, 60097};
static inline uint32_t exp8(uint32_t a, uint8_t e) {
	uint8_t es = e >> 3;
	uint8_t ei = e & 7;
	return (uint32_t) ((a * (uint64_t) EXP_EIGHTS[ei]) >> (15 - es));
}

#if !DEST_DUST
const uint16_t DIST_VALUES[] = {2534,12866,29178,37298,36555,33714,30919,28256,24735,17411,7276,1401};
static inline uint32_t normalDist(uint32_t a, uint8_t i) {
	return (uint32_t) ((a * (uint64_t) DIST_VALUES[i]) >> 18);
}

#define DIST_LEFT 3
#define DIST_LENGHT 12
#endif

static void calculateWh(int16_t w, int16_t h, uint32_t* resaults) {
	uint32_t ill = w * (uint32_t)w * h;
	if (g_dust_correctThresholdFunction != 0) {
		ill = g_dust_correctThresholdFunction(ill);
	}
	//uint64_t ilr = (g_dust_correctThreshold * UINT64_C(160000) * h) >> 16;

#if DEST_DUST
	uint64_t ilr = UINT64_C(160000) * h;
	uint8_t started = 0;
	uint32_t mp = 0, cp = 0;
	uint32_t il = 146;
#endif
	for (int8_t j = 0; j < 64; j++) {
		uint32_t ir = exp8(160, j);
		uint64_t i2r = ir * (uint64_t) ir;
#if DEST_DUST
		uint64_t i2l = il * (uint64_t) il;
		if (i2r > ill && i2l < ilr) {
			if (started == 0) {
				started = 1;
				mp = ir;
				cp = (1 << 24) / ir;
			} else {
				mp = (mp * (uint64_t)DATA_DIV_1) >> 16;
				cp = (cp * (uint64_t)DATA_DIV_3) >> 16;
			}
			int32_t we = ir - il;
			if (i2l < ill) {
				we -= (int32_t)((ill - i2l) * (ir - il) / (i2r - i2l));
			}
			if (i2r > ilr) {
				we -= (int32_t)((i2r - ilr) * (ir - il) / (i2r - i2l));
			}
			if (we > 0) {
				uint8_t index = j >> 3;
				*(resaults + index) += (mp * we) >> 10;
				*(resaults + 8 + index) += (cp * we) >> 12;
			}
		}
		il = ir;
#else
		if (i2r > ill) {
			for (int8_t k = max(j - DIST_LEFT,0); k < min(j + DIST_LENGHT - DIST_LEFT,64); k++)
			{
				uint8_t index = k >> 3;
				*(resaults + index) += normalDist(ill >> 10, k+DIST_LEFT-j);
				*(resaults + 8 + index) += normalDist(3000, k+DIST_LEFT-j);
			}
			break;
		}
#endif
	}
}


#define cache_push(a, v) (a.cache[a.cacheIndex++ & 0x03] = v)
#define cache_get(a, i) (a.cache[(a.cacheIndex-i)&0x03])

static void accumulate(dust_t* dust, int16_t* buffer) {
	uint8_t status = dust->wave.status;
	int16_t trigger = dust->config.trigger;
 	fortas(int16_t, buffer, dust->ac.size/2) {
		if (status == WAVE_LOW) {
			if (v > trigger && v > cache_get(dust->wave, 1)) {
				status = WAVE_RISE;
			}
		} else if (status == WAVE_HIGH) {
			if (v < trigger) {
				status = WAVE_FALL;
			}
			if ((v - trigger) < (dust->wave.height - (dust->wave.height >> 2) - 0x100)) {
				// falling
				if ((cache_get(dust->wave, 1) - v) * 2 < (cache_get(dust->wave, 2) - cache_get(dust->wave, 1)) - 0x30) {
					status = WAVE_FALL;
				}
			} else {
				if (((v - cache_get(dust->wave, 1)) > 2 * dust->wave.slope + 0x10)) {
					status = WAVE_RISE;
				}
			}
		}

		if (status == WAVE_RISE) {
			dust->wave.height = max(0, v - trigger);
			dust->wave.width = 1;
			dust->wave.topped = 0;
			dust->wave.slope = max(0, v - cache_get(dust->wave, 1));
			status = WAVE_HIGH;
		}
		else if (status == WAVE_HIGH) {
			dust->wave.height = max(dust->wave.height, v-trigger);
			dust->wave.width++;
			if (v > (UINT16_C(1) << 12) - 15) {
				dust->wave.topped++;
			}
		}
		else if (status == WAVE_FALL) {
			int16_t width = dust->wave.width, height = dust->wave.height;
			int16_t topped = dust->wave.topped, slope = dust->wave.slope;
			if (topped > 1) { // 20170110
				height = min(((slope * (int32_t)topped * topped) >> 6) + height, INT16_MAX); //0x7fff
			}
			int32_t rs = dust->wave.function(dust, width, height);
			if (rs > 0) {
				foris(j, 4) {
					dust->accumulate.proportion[j] += (rs > dust->proportion.pmas[j].current) ? 1 : 0;
				}
				dust->accumulate.proportion[4] += 1;
			}
			status = WAVE_LOW;
		}
		cache_push(dust->wave, v);
	}
	dust->wave.status = status;
}
static void aggregate(dust_t* dust) {
	forta(uint32_t, dust->accumulate.pm) {
		for (int8_t j = max(0,i-3); j < 4; j ++) {
			dust->aggregate.pm[j] += v;
		}
	}
	forta(uint32_t, dust->accumulate.pc) {
		for (int8_t j = 0; j < max(1,i + 5 - 8 + 1); j ++) {
			uint32_t addvalue;
			if (j != 0) {
				addvalue = v << (j * 2 + 2);
			} else {
				addvalue = v;
			}
			dust->aggregate.pc[j] += addvalue;
		}
	}
	for (uint8_t i = 0; i<sizeof(dust->proportion.pmas)/sizeof(average_pma_t); i++) {
		average_pma_push(&dust->proportion.pmas[i], dust->accumulate.proportion[4], dust->accumulate.proportion[i]);
	}
	dust->aggregate.count = +dust->accumulate.count;
	memset(&dust->accumulate, 0, sizeof(dust->accumulate));
}
static uint8_t kCheck(uint8_t i, average_sma_t* sma, uint8_t size, uint32_t average, uint64_t noise) {
	if (sma->length >= size) {
		int32_t right = average_sma_average(sma, 0, size >> 1, 1) >> DATA_SHIFT;
		int32_t left = average_sma_average(sma, size >> 1, size, 1) >> DATA_SHIFT;
		average = average >> DATA_SHIFT;
		noise = noise >> (DATA_SHIFT << 1);
		uint32_t ad;
		if ((right - left) < 0) {
			ad = left - right;
		} else {
			ad = (right - left);
		}
		if ((uint64_t)(ad * ad) * (uint16_t)(1 << i) * average * average > (18 - 5) * noise * (right + left) * (right + left)) {
			return 1;
		}
	}
	return 0;
}
static void calculate(dust_t* dust) {
	// uint32_t source = dust->aggregate.pm[1]*ACCUMULATE_COUNT_TO_CALCULATE/dust->accumulate.count;
	uint32_t source = dust->aggregate.pm[1];
	average_cma_push(&dust->calculate.cma4, source, 1);
	average_sma_push(&dust->calculate.sma4, source);
	if (dust->calculate.cma4.cnt == 40) {
		average_sma_push(&dust->calculate.sma128, average_cma_get(&dust->calculate.cma4, 1));
		dust->calculate.average128 = average_sma_average(&dust->calculate.sma128, 0, dust->calculate.sma128.length, 1);
		if (dust->calculate.sma128.length > 2) {
			dust->calculate.noise128 = average_sma_noise(&dust->calculate.sma128);
			uint64_t maxNoise = dust->calculate.average128  * (uint64_t) dust->calculate.average128 / 40;
			maxNoise = max(maxNoise, dust->calculate.average128 * UINT64_C(500));
			maxNoise = min(maxNoise, dust->calculate.average128 * (uint64_t) dust->calculate.average128 / 10 );
			if (dust->calculate.noise128 > maxNoise) {
				dust->calculate.noise128 = maxNoise;
			}
			for (uint8_t i = 3; i < 8; i++) {
				uint8_t n = 1 << i;
				uint8_t kgn = kCheck(i, &dust->calculate.sma128, n >> 2, dust->calculate.average128, dust->calculate.noise128 * 4);
				if (kgn) {
					dust->calculate.kgns |= (1 << i);
				} else {
					dust->calculate.kgns &= ~(1 << i);
				}
			}
		}
	}

	uint32_t average4 = average_sma_average(&dust->calculate.sma4, 0, dust->calculate.sma4.length, 1);

	if (dust->calculate.noise128 > 0) { //dust->calculate.sma4.length > 2
		uint64_t sma4n = average_sma_noise(&dust->calculate.sma4);
		for (uint8_t i = 0; i < 3; i++) {
			uint8_t n = 1 << i;
			uint8_t kgn = kCheck(i, &dust->calculate.sma4, n * 10, dust->calculate.sma128.length > 8 ? dust->calculate.average128 : average4, dust->calculate.sma128.length > 8 ? dust->calculate.noise128 * 4 : sma4n / 10);
			//uint8_t kgn = kCheck(i, &dust->calculate.sma4, n * 10, dust->calculate.average128, dust->calculate.noise128 * 4);

			if (kgn) {
				dust->calculate.kgns |= (1 << i);
			} else {
				dust->calculate.kgns &= ~(1 << i);
			}
		}
	}
	uint16_t fallSpeed = 0;
	// #ifdef DUST_DUO
	// uint16_t fallGround = 400;
	// #else
	uint16_t fallGround = 12800;
	// #endif
	for (uint8_t i = 0; i < 8; i++) {
		if (dust->calculate.kgns & (1 << i)) {
			fallSpeed += (12800 >> (i + 1));
			fallGround = min(fallGround, 50 << i);
		}
	}
	fallGround = max(fallGround, dust->config.minAverageLength);

	if (dust->calculate.averageLength >= fallGround) {
		dust->calculate.averageLength = max(dust->calculate.averageLength - fallSpeed * (uint32_t)dust->calculate.averageLength * 5 / 128000, fallGround);
	} else {
		dust->calculate.averageLength += 12;
	}

	fors(4) {
		//*ACCUMULATE_COUNT_TO_CALCULATE/dust->accumulate.count
		dust->out.pm[i] = average_ema_shift(dust->out.pm[i], dust->aggregate.pm[i], 7);
	}
	fors(5) {
		//*ACCUMULATE_COUNT_TO_CALCULATE/dust->accumulate.count
		dust->out.pc[i] = average_ema_shift(dust->out.pc[i], dust->aggregate.pc[i], 7);
	}
	memset(&dust->aggregate, 0, sizeof(dust->aggregate));
	for (uint8_t i = 0; i<sizeof(dust->proportion.pmas)/sizeof(average_pma_t); i++) {
		if (dust->proportion.pmas[i].all >= PMA_SUM_LIMIT && dust->proportion.pmas[i].matched >= PMA_MATCH_LIMIT || dust->proportion.pmas[i].all >= 2*PMA_SUM_LIMIT ) {
			average_pma_correct(&dust->proportion.pmas[i], 15-i);
			//average_pma_correct(dust->proportion.pmas[i], 12);
		}
	}
}
static void output(dust_t* dust) {
#if !RAW_DUST
	uint16_t cma4c10 = dust->calculate.cma4.cnt * 10;
	uint32_t raw;
	if (dust->calculate.averageLength >= cma4c10 + 400) {
		uint16_t gal = dust->calculate.averageLength - cma4c10;
		uint64_t sum = 0;
		uint32_t cnt = 0;
		uint16_t size = min(gal / 400, dust->calculate.sma128.length);
		sum += average_sma_average(&dust->calculate.sma128, 0, size, size) * UINT64_C(400);
		cnt += 400 * size;
		if (size < dust->calculate.sma128.length) {
			uint16_t w = gal - size * 400;
			sum += average_sma_get(&dust->calculate.sma128, size) * w;
			cnt += w;
		}
		if (cma4c10 > 0) {
			sum += average_cma_peek(&dust->calculate.cma4, 1) * cma4c10;
			cnt += cma4c10;
		}
		raw = (uint32_t)((sum + (cnt >> 1)) / max(cnt,1));
	} else {
		uint8_t size = min(dust->calculate.averageLength / 10, dust->calculate.sma4.length);
		raw = average_sma_average(&dust->calculate.sma4, 0, size, 1);
	}
	int32_t diver = dust->out.pm[DATA_PM2D5_INDEX] + 100;
	// *(buffer + DUST_OUTPUT_DATA_PM2D5) = raw;
	dust->calibrate.pm[1] = raw;
	fors (4) {
		if (i != DATA_PM2D5_INDEX) {
			uint32_t value = ((dust->out.pm[i] + 100) * (uint64_t)raw + (diver >> 1)) / diver; // (uint64_t)
			// *(buffer + DUST_OUTPUT_DATA_PM1D0 + i) = value;
			dust->calibrate.pm[i] = value;
		}
	}
	fors (5) {
		uint32_t value = ((dust->out.pc[i] + 100) * (uint64_t)raw + (diver >> 1)) / diver; // (uint64_t)
		if (i>0) {
			value = value >> (i * 2 + 2);
		}
		// *(buffer + DUST_OUTPUT_DATA_PC0D3 + i) = value;
		dust->calibrate.pc[i] = value;
	}
	{
		uint64_t sum = 0;
		uint32_t cnt = 0;
		sum += dust->calculate.average128 * (uint64_t)dust->calculate.sma128.length * 40;
		cnt += dust->calculate.sma128.length * 40;
		sum += dust->calculate.cma4.sum;
		cnt += dust->calculate.cma4.cnt;
		// *(buffer + DUST_OUTPUT_DATA_SMA128) = cnt == 0?0:(sum + (cnt >> 1)) / max(cnt,1);
		dust->calibrate.sma128 = cnt == 0?0:(sum + (cnt >> 1)) / max(cnt,1);
	}
	// *(buffer + DUST_OUTPUT_DATA_SMA4) = average4;
	// *(buffer + DUST_OUTPUT_NOISE4L) = dust->calculate.noise128;
	// *(buffer + DUST_OUTPUT_NOISE4H) = dust->calculate.noise128 >> 32;
	// *(buffer + DUST_OUTPUT_AVERAGE_LENGTH) = dust->calculate.averageLength;
	// dust.calibrate.averageLength = dust->calculate.averageLength;
  //
	// fors (4) {
	// 	// *(buffer + DUST_OUTPUT_PROPORTION_2000 + i) = dust->proportion.pmas[i].current;
	// 	dust.calibrate.proportion[i] = dust->proportion.pmas[i].current;
	// }
#endif
}

static void calibrate(dust_t* dust, uint32_t* buffer) {
	int32_t pm25 = dust->processPmFunction(dust, dust->calibrate.pm[1]);
	if (pm25<dust->config.asZero) {
		fors (4) {
			*(buffer + DUST_OUTPUT_DATA_PM1D0 + i) = 0;
		}
		fors (5) {
			*(buffer + DUST_OUTPUT_DATA_PC0D3 + i) = 0;
		}
	} else {
		fors (4) {
			*(buffer + DUST_OUTPUT_DATA_PM1D0 + i) = dust->processPmFunction(dust, dust->calibrate.pm[i]);
		}
		fors (5) {
			*(buffer + DUST_OUTPUT_DATA_PC0D3 + i) = dust->processPcFunction(dust, dust->calibrate.pc[i]);
		}
	}
	// *(buffer + DUST_OUTPUT_DATA_SMA128) = dust.calibrate.sma128;
	// *(buffer + DUST_OUTPUT_AVERAGE_LENGTH) = dust.calibrate.averageLength;
}

static void sampling(void** params) {
	dust_t* dust = (dust_t*) params;
	uint32_t now = tick_ms();
	//TODO laser off and on
	int16_t* buffer = dust_hw_getAcData(dust);
	if (buffer) {
		accumulate(dust,buffer);
		dust->accumulate.count ++;
		if (dust->accumulate.count == 100) {
			aggregate(dust);
			calculate(dust);
			uint32_t interval;
			uint32_t al = dust->calculate.averageLength;
			if (al > 1600) {
				interval = 2000;
			} else if (al < 400) {
				interval = 200;
			} else {
				interval = 200 + (al - 400)/200 * 100;
			}
			if (now - (dust->ticks.out + interval) < INT32_MAX) {
				uint32_t out[DUST_OUTPUT_SIZE];
				output(dust);
				calibrate(dust, out);
				dust->dataFunction(dust, out);
				dust->ticks.out = now;
			}
		}
		dust_hw_dropAcData(dust, buffer);
	} else {
		__nop();
	}
}

void dust_init(dust_t* dust) {
	dust_hw_init(dust);
	// schedule_repeat((uint32_t) dust, sampling, 1, (void**)dust);
	// dust_hw_fan(1);
	// dust_hw_laser(1);
}


void dust_enable(dust_t* dust, uint8_t enable) {
	if (enable) {
		schedule_repeat((uint32_t) dust, sampling, 1, (void**)dust);
	} else {
		schedule_cancel((uint32_t) dust);
	}
	dust_hw_fan(dust, enable);
}


void dust_laser(dust_t* dust, uint8_t enable) {
	dust_hw_laser(dust, enable);
}
