#ifndef NUMBER_H__
#include <stdint.h>
#define NUMBER_ENABLE_DOUBLE 0

#if NUMBER_ENABLE_DOUBLE
typedef int64_t intFraction_t;
#define NUMBER_FRACTION_C(a) INT64_C(a)
#define NUMBER_FRACTION_BTYES 8
#else
typedef int32_t intFraction_t;
#define NUMBER_FRACTION_C(a) INT32_C(a)
#define NUMBER_FRACTION_BTYES 4
#endif

//typedef void(*number_byteFunction_t)(uint8_t);

#define max(a,b) ((a)>(b)?(a):(b))
#define min(a,b) ((a)<(b)?(a):(b))
#define abs(a) ((a)<0?(-(a)):(a))

uint8_t number_bitGet(uint8_t const* buffer, int16_t pos);
uint8_t number_bitGetBE(uint8_t const* buffer, int16_t pos);
void number_bitSet(uint8_t* buffer, int16_t pos);
void number_bitReset(uint8_t* buffer, int16_t pos);

int16_t number_toDecString(int32_t value, int8_t dotPosition, char* buffer, int16_t bufferSize);
int16_t number_fromDecString(const char* buffer, int16_t bufferSize, int32_t* value, int8_t* dotPosition);

int16_t number_toHexString(uint32_t value, char* out, int16_t bufferSize, uint8_t bigLetter);
int32_t number_fromHexString(const char* buffer, int16_t size);

int32_t number_valueInt32(uint8_t const* buffer, int16_t size);
int64_t number_valueInt64(uint8_t const* buffer, int16_t size);

int16_t number_minBytes(uint8_t* buffer, int16_t size);
int16_t number_minBits(uint8_t* buffer, int16_t size);

int32_t number_e10(int16_t e);
int32_t number_int(int32_t value, int8_t e10);

void number_toFloat(uint8_t sign, int32_t exponent, intFraction_t fraction, uint8_t* buffer, int16_t size);
void number_fromFloat(uint8_t* sign, int32_t* exponent, intFraction_t* fraction, uint8_t const* buffer, int16_t size);

int16_t number_floatMinBytes(uint8_t sign, int32_t exponent, intFraction_t fraction);

int32_t number_circleCompare(int32_t a, int32_t b, uint32_t size);

#define NUMBER_H__
#endif
