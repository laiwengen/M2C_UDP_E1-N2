#ifndef DUST_H__
#define DUST_H__
#include "stdint.h"
#include "main.h"
#include "frameworks/average.h"

#define RAW_SAMPLE 0
#define DEST_SAMPLE 1
#define PMA_START 400000

enum dust_outputDatas_t{
	DUST_OUTPUT_DATA_PM1D0,
	DUST_OUTPUT_DATA_PM2D5,
	DUST_OUTPUT_DATA_PM5D0,
	DUST_OUTPUT_DATA_PM10,
	DUST_OUTPUT_DATA_PC0D3,
	DUST_OUTPUT_DATA_PC1D0,
	DUST_OUTPUT_DATA_PC2D5,
	DUST_OUTPUT_DATA_PC5D0,
	DUST_OUTPUT_DATA_PC10,
	// DUST_OUTPUT_DATA_SMA128,
	// DUST_OUTPUT_DATA_SMA4, //
	// DUST_OUTPUT_NOISE4L, //
	// DUST_OUTPUT_NOISE4H, //
	// DUST_OUTPUT_AVERAGE_LENGTH,
	// DUST_OUTPUT_PROPORTION_2000,
	// DUST_OUTPUT_PROPORTION_600,
	// DUST_OUTPUT_PROPORTION_50,
	// DUST_OUTPUT_PROPORTION_10,
	DUST_OUTPUT_SIZE,
};

typedef struct dust_t dust_t;
// typedef int16_t (*dust_)

typedef int32_t (*dust_waveFunction_t) (dust_t*,int16_t,int16_t);
typedef uint32_t (*dust_processFunction_t) (dust_t*, uint32_t);
typedef void (*dust_dataFunction_t) (dust_t*, uint32_t*);

struct dust_t {
	#ifdef DUST_DUO
	uint32_t id;
	#endif
	struct {
		int16_t trigger;
		int16_t minAverageLength;
		int16_t asZero;
	} config;
	struct {
		uint8_t* buffers[2];
		int16_t size;
		uint8_t ids[2];
	} ac;
	struct {
		dust_waveFunction_t function;
		int16_t height;
		int16_t width;
		int16_t topped;
		int16_t slope;
		int16_t cache[4];
		int8_t cacheIndex;
		uint8_t status;
	} wave;
	struct {
		uint32_t pm[8];
		uint32_t pc[8];
		int16_t count;
		uint16_t proportion[5];
	} accumulate;
	struct {
		uint32_t pm[4];
		uint32_t pc[5];
		int16_t count;
	} aggregate;
	struct {
		average_pma_t pmas[4];
	} proportion;
	struct {
		average_cma_t cma4;
		average_sma_t sma4;
		average_sma_t sma128;
		uint32_t averageLength;
		uint32_t average128;
		uint64_t noise128;
		uint16_t kgns;
	} calculate;
	struct {
		uint32_t pm[4];
		uint32_t pc[5];
	} out;
	struct {
		uint32_t pm[4];
		uint32_t pc[5];
		uint32_t sma128;
		// uint32_t averageLength;
		// uint32_t proportion[4];
	} calibrate;
	struct {
		uint32_t out;
	} ticks;
	dust_dataFunction_t dataFunction;
	dust_processFunction_t processPmFunction;
	dust_processFunction_t processPcFunction;
};


void dust_enable(dust_t* dust, uint8_t enable);
void dust_laser(dust_t* dust, uint8_t enable);
void dust_init(dust_t* dust);

int16_t* dust_hw_getAcData(dust_t* dust);
void dust_hw_dropAcData(dust_t* dust, int16_t* a);
void dust_hw_fan(dust_t* dust, uint8_t enable);
void dust_hw_laser(dust_t* dust, uint8_t enable);
void dust_hw_init(dust_t* dust);

#endif
